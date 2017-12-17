#include "ConnectionManager.h"

WiFiManager ConnectionManager::_client; 

std::forward_list<WiFiManagerParameter> ConnectionManager::_params;
 
std::unordered_map<std::string, std::string> ConnectionManager::_values;

cm_callback_t ConnectionManager::_connect, 
    ConnectionManager::_disconnect,
    ConnectionManager::_autoconnect
;

void ConnectionManager::erase() {
    _autoconnect = []() {};
    WiFi.disconnect();
}

void ConnectionManager::parameter(const char* text, const char* placeholder, size_t buf) {
    // Read from file before emplace so we can show the current value if exists
    if( _values.empty() ) _read_config();
    
    _params.emplace_front(text, placeholder, parameter(text).c_str(), buf);
    _client.addParameter(&_params.front()); 
} 

std::string ConnectionManager::parameter(const char* text) {
    auto i = _values.find(std::string(text));
    if(i == _values.end()) return ""; 
    else return i->second;
}


cm_err_t ConnectionManager::_read_config(){ 

    // Open file for reading
    file_ptr_t filePtr;
    cm_err_t open_result = _open_file("r", filePtr);
    if(open_result != CM_OK) return open_result;
 
    // Read key value pairs into temporary storage
    std::unordered_map<std::string, std::string> temp;
    
    while( filePtr->available() ) {
        temp.emplace(
            filePtr->readStringUntil('\n').c_str(),
            filePtr->readStringUntil('\n').c_str()
        ); 
    }
    filePtr->close();

    _values.clear();
    _values = temp;
   
    return CM_OK;

}

cm_err_t ConnectionManager::_save_config(){

    // Load WiFiManagerParameter results into forward_list
    _values.clear();
    for(auto i : _params) {
       _values.emplace( i.getID(), i.getValue() );
    }
       
    // Open the config file for writing
    file_ptr_t filePtr;
    cm_err_t open_result = _open_file("w", filePtr);
    if(open_result != CM_OK) return open_result;

    // Write key value pairs
    size_t written = 0;
    for(auto i : _values) {
        written += filePtr->print(i.first.c_str());
        written += filePtr->print('\n');
        written += filePtr->print(i.second.c_str());
        written += filePtr->print('\n');
    }
    filePtr->close();

    if(filePtr->getWriteError()) return CM_WRITE_ERROR;

    return CM_OK;
}

cm_err_t ConnectionManager::_open_file(const char* mode, file_ptr_t &filePtr){
    
    // Make sure file is ready to be opened
    if( !SPIFFS.begin() ) return CM_OPEN_ERROR;
    if( !SPIFFS.exists(CM_FILE) ) {
        SPIFFS.open(CM_FILE, "w").close();
    }
 
    // Create a new file object by opening from SPIFFS
    filePtr.reset(new File);
    *filePtr = SPIFFS.open(CM_FILE, mode);

    // Make sure the open worked
    if(! *filePtr || !filePtr) {
        return CM_OPEN_ERROR;
    }

    return CM_OK;
}


