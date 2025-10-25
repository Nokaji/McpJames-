#pragma once
#include "transport.hpp"
#include "type/mcp_type.hpp"
#include <httplib.h>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <regex>
#include <sstream>
#include <iostream>

namespace mcp {

class SseTransport : public Transport {
    std::atomic<bool> running{false};
    std::atomic<bool> connected{false};  // Nouveau : indique si la connexion SSE est active
    std::thread listener;
    std::shared_ptr<httplib::Client> client;

    type::SseConfig config;
    std::string sessionId;
    std::string lastEventId;
    std::mutex sessionMutex;
    std::condition_variable connectionCV;  // Nouveau : pour attendre la connexion

    // Buffer pour accumuler les données SSE fragmentées
    std::string sseBuffer;

    // Parse les événements SSE selon le standard W3C
    void parseSSEMessage(const std::string& rawData, const MessageHandler& onMessage) {
        // Ajouter au buffer
        sseBuffer += rawData;
        
        size_t pos = 0;
        while ((pos = sseBuffer.find("\n\n")) != std::string::npos) {
            std::string event = sseBuffer.substr(0, pos);
            sseBuffer.erase(0, pos + 2);
            
            if (!event.empty()) {
                processSSEEvent(event, onMessage);
            }
        }
    }
    
    void processSSEEvent(const std::string& eventBlock, const MessageHandler& onMessage) {
        std::istringstream stream(eventBlock);
        std::string line;
        std::string eventType = "message"; // Type par défaut selon spec SSE
        std::string data;
        std::string eventId;
        
        while (std::getline(stream, line)) {
            // Supprimer \r si présent
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            // Ignorer les lignes vides et les commentaires
            if (line.empty() || line[0] == ':') {
                continue;
            }
            
            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) {
                continue;
            }
            
            std::string field = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Trim le premier espace après le : selon spec SSE
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            
            if (field == "event") {
                eventType = value;
            } else if (field == "data") {
                if (!data.empty()) {
                    data += "\n";
                }
                data += value;
            } else if (field == "id") {
                eventId = value;
            }
        }
        
        // Sauvegarder l'ID de l'événement pour reconnexion
        if (!eventId.empty()) {
            lastEventId = eventId;
        }
        
        // Traiter l'événement
        handleEvent(eventType, data, onMessage);
    }
    
    void handleEvent(const std::string& eventType, const std::string& data, const MessageHandler& onMessage) {
        if (data.empty()) {
            return;
        }
        
        std::cout << "[SSE Transport] Event: " << eventType << std::endl;
        
        if (eventType == "endpoint") {
            // Extraire le sessionId de l'URL endpoint
            std::regex sessionRegex(R"(\?sessionId=([a-zA-Z0-9\-]+))");
            std::smatch match;
            if (std::regex_search(data, match, sessionRegex)) {
                {
                    std::lock_guard<std::mutex> lock(sessionMutex);
                    sessionId = match[1].str();
                    connected.store(true);  // Marquer comme connecté
                    std::cout << "[SSE Transport] ✓ Captured sessionId: " << sessionId << std::endl;
                    std::cout << "[SSE Transport] Endpoint URL: " << data << std::endl;
                }
                connectionCV.notify_all();  // Notifier les threads en attente
            }
        } else if (eventType == "message" || eventType.empty()) {
            // Message JSON-RPC standard
            onMessage(data);
        } else {
            // Autres types d'événements (pour extensions futures)
            std::cout << "[SSE Transport] Unknown event type: " << eventType << std::endl;
            std::cout << "[SSE Transport] Data: " << data << std::endl;
        }
    }

public:
    explicit SseTransport(const type::SseConfig& config)
        : config(config) {}

    ~SseTransport() override {
        stop();
    }

    // Supprimer copie, permettre move
    SseTransport(const SseTransport&) = delete;
    SseTransport& operator=(const SseTransport&) = delete;
    SseTransport(SseTransport&& other) noexcept = default;

    void setSessionId(const std::string& sid) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        sessionId = sid;
    }

    std::string getSessionId() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(sessionMutex));
        return sessionId;
    }

    // Attendre que la connexion SSE soit établie et que le sessionId soit reçu
    bool waitForConnection(int timeoutMs = 10000) {
        std::unique_lock<std::mutex> lock(sessionMutex);
        return connectionCV.wait_for(lock, std::chrono::milliseconds(timeoutMs), 
                                      [this] { return connected.load(); });
    }

    void send(const std::string& message) override {
        // Attendre que la connexion soit établie avant d'envoyer
        if (!connected.load()) {
            std::cout << "[SSE Transport] Waiting for connection before sending..." << std::endl;
            if (!waitForConnection(10000)) {
                std::cout << "[SSE Transport] ERROR: Connection timeout - cannot send message" << std::endl;
                return;
            }
        }
        
        httplib::Client cli(config.url);
        cli.set_connection_timeout(5, 0); // 5 secondes
        cli.set_write_timeout(5, 0);
        cli.set_read_timeout(5, 0);
        
        std::string currentSessionId;
        {
            std::lock_guard<std::mutex> lock(sessionMutex);
            currentSessionId = sessionId;
        }
        
        // Construire l'URL avec sessionId si disponible
        std::string endpoint = config.messageEndpoint;
        if (!currentSessionId.empty()) {
            endpoint += "?sessionId=" + currentSessionId;
        }
        
        // Headers pour JSON-RPC
        httplib::Headers headers = {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"}
        };
        
        // Ajouter headers personnalisés depuis config
        for (const auto& [key, value] : config.headers) {
            headers.emplace(key, value);
        }
        
        std::cout << "[SSE Transport] POST to: " << config.url << endpoint << std::endl;
        auto res = cli.Post(endpoint.c_str(), headers, message, "application/json");
        
        if (res) {
            std::cout << "[SSE Transport] POST response status: " << res->status << std::endl;
            if (res->status >= 400) {
                std::cout << "[SSE Transport] Error response: " << res->body << std::endl;
            }
        } else {
            std::cout << "[SSE Transport] POST failed - Error: " << httplib::to_string(res.error()) << std::endl;
        }
    }

    void start(MessageHandler onMessage) override {
        if (running.load()) {
            std::cout << "[SSE Transport] Already running" << std::endl;
            return;
        }
        
        running = true;
        listener = std::thread([this, onMessage]() {
            client = std::make_shared<httplib::Client>(config.url);
            
            // Configuration des timeouts
            client->set_connection_timeout(10, 0); // 10 secondes pour la connexion
            client->set_read_timeout(0, 500000); // 500ms pour permettre l'arrêt régulier
            
            // Désactiver la compression pour SSE
            client->set_compress(false);
            
            int attemptCount = 0;
            const int maxAttempts = config.maxRetries > 0 ? config.maxRetries : -1; // -1 = infini
            
            while (running.load() && (maxAttempts == -1 || attemptCount < maxAttempts)) {
                try {
                    std::cout << "[SSE Transport] Connecting to SSE endpoint: " 
                              << config.url << config.sseEndpoint;
                    
                    // Headers SSE standard
                    httplib::Headers headers = {
                        {"Accept", "text/event-stream"},
                        {"Cache-Control", "no-cache"},
                        {"Connection", "keep-alive"}
                    };
                    
                    // Ajouter Last-Event-ID pour reconnexion
                    if (!lastEventId.empty()) {
                        headers.emplace("Last-Event-ID", lastEventId);
                        std::cout << " (Last-Event-ID: " << lastEventId << ")";
                    }
                    std::cout << std::endl;
                    
                    // Ajouter headers personnalisés
                    for (const auto& [key, value] : config.headers) {
                        headers.emplace(key, value);
                    }
                    
                    // Réinitialiser le buffer
                    sseBuffer.clear();
                    
                    auto res = client->Get(
                        config.sseEndpoint.c_str(),
                        headers,
                        [&](const char* data, size_t len) {
                            if (!running.load()) {
                                return false;
                            }
                            
                            std::string chunk(data, len);
                            if (!chunk.empty()) {
                                parseSSEMessage(chunk, onMessage);
                            }
                            // Continuer à lire les données SSE
                            return true;
                        }
                    );
                    
                    if (!running.load()) {
                        std::cout << "[SSE Transport] Stopped by user" << std::endl;
                        break;
                    }
                    
                    // Marquer comme déconnecté pour forcer l'attente à la prochaine reconnexion
                    connected.store(false);
                    
                    if (!res) {
                        std::cout << "[SSE Transport] Connection error: " 
                                  << httplib::to_string(res.error()) << std::endl;
                    } else if (res->status != 200) {
                        std::cout << "[SSE Transport] HTTP error " << res->status 
                                  << ": " << res->body << std::endl;
                    } else {
                        // La connexion s'est terminée normalement (serveur a fermé)
                        std::cout << "[SSE Transport] Connection closed by server" << std::endl;
                    }
                    
                    attemptCount++;
                    
                    // Reconnexion avec backoff exponentiel
                    if (running.load() && (maxAttempts == -1 || attemptCount < maxAttempts)) {
                        int delay = std::min(config.reconnectDelayMs * (attemptCount > 1 ? attemptCount : 1), 30000); // Max 30s
                        std::cout << "[SSE Transport] Reconnecting in " << delay << "ms (attempt " 
                                  << attemptCount << ")" << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                    }
                    
                } catch (const std::exception& e) {
                    std::cout << "[SSE Transport] Exception: " << e.what() << std::endl;
                    if (!running.load()) {
                        break;
                    }
                    attemptCount++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(config.reconnectDelayMs));
                }
            }
            
            std::cout << "[SSE Transport] Listener thread exiting" << std::endl;
        });
    }

    void stop() override {
        if (!running.load()) {
            return;
        }
        
        std::cout << "[SSE Transport] Stopping..." << std::endl;
        running = false;
        connected.store(false);
        
        // Réveiller tous les threads en attente
        connectionCV.notify_all();
        
        // Arrêter le client pour débloquer le thread
        if (client) {
            client->stop();
        }
        
        // Attendre la fin du thread
        if (listener.joinable()) {
            listener.join();
        }
        
        std::cout << "[SSE Transport] Stopped" << std::endl;
    }
};

} // namespace mcp