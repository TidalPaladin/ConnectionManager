/**
 * ConnectionManager
 * Scott Chase Waggener
 * 12/16/17
 * 
 * This library extends WiFiManager.
 * 
 * Connection management and custom parameters are handled in one place. 
 * 
 */

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H


#include <Arduino.h>
#include <FS.h> 
#include <WiFiManager.h>

#include <forward_list>
#include <unordered_map>
#include <fstream>
#include <iterator>
#include <sstream>
#include <memory>

#define CM_FILE         "/config.json"
#define CM_DEFAULT_VAL  ""

typedef enum {
    CM_OK = 0,
    CM_NO_FILE,
    CM_OPEN_ERROR,
    CM_WRITE_ERROR,
    CM_READ_ERROR,
    CM_WM_ERROR
} cm_err_t;
 
typedef std::function<void()> cm_callback_t;
 
class ConnectionManager {

public:    

    /**
     * @brief Binds WiFi connect and disconnect events and
     * starts the connection
     */
    // template<typename... Args>
    // static cm_err_t autoConnect(Args&&... args) { 
    //     // Setup callbacks
    //     _client.setSaveConfigCallback(reinterpret_cast<void(*)(void)>(_save_config));
    //     WiFi.onStationModeConnected(ConnectionManager::_on_connect);
    //     WiFi.onStationModeDisconnected(ConnectionManager::_on_disconnect); 

    //     // Bind autoconnection parameters and connect
    //     _autoconnect = [args...]() {
    //          _client.autoConnect(std::forward<Args>(args)...); 
    //     }; 
        
    //     //_autoconnect();

    //     return ( _values.empty() ) ? _read_config() : CM_OK;

    // }
    static cm_err_t autoConnect(const char* ssid) {
        _client.setSaveConfigCallback(reinterpret_cast<void(*)(void)>(_save_config));

        // Dont let these handlers get deleted or callbacks wont run!
        static auto h1 = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected &c) {
            if(_connect) _connect();
        });

        static auto h2 = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &c) {
            if(_disconnect) _disconnect();
            if(_autoconnect) _autoconnect(); // Reconnect
        });

        // Bind autoconnection parameters and connect
        _autoconnect = [&ssid]() {
            static bool running;
            if(running) return;
            running = true;
            _client.autoConnect(ssid); 
            running = false;
        }; 
        _autoconnect();

        return ( _values.empty() ) ? _read_config() : CM_OK;

    }

    /**
     * @brief Erases stored WiFi settings
     * 
     */
    static void erase();

    /**
     * @brief Adds a custom WiFiManager parameter
     * 
     * @param text The name of the parameter
     * @param placeholder The placeholder text used in the config portal
     * @param buf The buffer size for the incoming data
     * 
     * @return this
     */
    static void parameter(const char* text, const char* placeholder, size_t buf = 64);
    

    /**
     * @brief Retrivies a WiFiManager parameter by name from storage
     * 
     * @param text The name of the parameter
     * 
     * @return 
     */
    static std::string parameter(const char* text);


    /**
     * @brief Sets a callback to run on connect / disconnect. Attempting
     * to reconnect to wifi is handled automatically
     * 
     * @param f The function to be run
     * 
     * 
     */
    static void onConnect(cm_callback_t f) { _connect = f; }
    static void onDisconnect(cm_callback_t f) { _disconnect = f; }

    /**
     * @brief Sets a callback to run on WiFiManager config portal
     * 
     */
    //static void onPortal(std::function<void(WiFiManager*)> f) { _client.setAPCallback(f); }

private: 

    typedef std::unique_ptr<File> file_ptr_t;

    // Dont allow creation of instances
    ConnectionManager() {}

    // Holds the WiFiManagerParameter objects
    // Emptied upon autoconnect
    static std::forward_list<WiFiManagerParameter> _params;

    // Holds just the ID / value pairs
    static std::unordered_map<std::string, std::string> _values;

    static cm_callback_t _connect, _disconnect, _autoconnect;

    static WiFiManager _client;


    /**
     * @brief Writes WiFiManager parameters to SPIFFS
     * 
     * @return cm_err_t
     */
    static cm_err_t _save_config();

    /**
     * @brief Reads WiFiManager parameters from SPIFFS to an unordered
     * map
     * 
     * @return cm_err_t
     */
    static cm_err_t _read_config();

    /**
     * @brief Opens a config file in a parameter defined mode.
     * 
     * @param mode The file mode ("w", "r")
     * @param filePtr A smart pointer to the opened file
     */
    static cm_err_t _open_file(const char* mode, file_ptr_t &filePtr);


};



#endif