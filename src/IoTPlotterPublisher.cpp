/**
 * @file IoTPlotterPublisher.cpp
 * @copyright 2017-2022 Stroud Water Research Center
 * Part of the EnviroDIY ModularSensors library for Arduino
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Implements the IoTPlotterPublisher class.
 */

#include "IoTPlotterPublisher.h"


// ============================================================================
//  Functions for the IoTPlotter data portal receivers.
// ============================================================================

// Constant values for post requests
// Example taken from https://iotplotter.com/docs/
// I want to refer to these more than once while ensuring there is only one copy
// in memory
const char* IoTPlotterPublisher::IoTPlotterHost       = "iotplotter.com";
const char* IoTPlotterPublisher::postEndpoint        = "/api/v2/feed/";
const int   IoTPlotterPublisher::IoTPlotterPort       = 80;  // LPM: Not necessary on IoTPlotter
const char* IoTPlotterPublisher::apiHeader         = "\r\napi-key: ";  // LPM: was TokenHeader
const char* IoTPlotterPublisher::contentTypeHeader =
    "\r\nContent-Type: application/x-www-form-urlencoded";
const char* IoTPlotterPublisher::contentLengthHeader = "\r\nContent-Length: ";


const char* IoTPlotterPublisher::samplingFeatureTag = "{\"data\":{\"";  // start of the JSON package
const char* IoTPlotterPublisher::JSONvalueTag = "\":[{\"value\":";
const char* IoTPlotterPublisher::epochTag       = ", \"epoch\":";        // LPM check this


// Constructors
IoTPlotterPublisher::IoTPlotterPublisher() : dataPublisher() {}
IoTPlotterPublisher::IoTPlotterPublisher(Logger& baseLogger, uint8_t sendEveryX,
                                       uint8_t sendOffset)
    : dataPublisher(baseLogger, sendEveryX, sendOffset) {}
IoTPlotterPublisher::IoTPlotterPublisher(Logger& baseLogger, Client* inClient,
                                       uint8_t sendEveryX, uint8_t sendOffset)
    : dataPublisher(baseLogger, inClient, sendEveryX, sendOffset) {}
IoTPlotterPublisher::IoTPlotterPublisher(Logger&     baseLogger,
                                       const char* apiKey,  // was registrationToken
                                       const char* feedID,  // was samplingFeatureUUID
                                       uint8_t sendEveryX, uint8_t sendOffset)
    : dataPublisher(baseLogger, sendEveryX, sendOffset) {
    setToken(apiKey);   // was registrationToken
    setFeedID(feedID);  
    _baseLogger->setSamplingFeatureUUID(feedID);       // was samplingFeatureUUID
}
IoTPlotterPublisher::IoTPlotterPublisher(Logger& baseLogger, Client* inClient,
                                       const char* apiKey,  // was registrationToken
                                       const char* feedID,  // was samplingFeatureUUID
                                       uint8_t sendEveryX, uint8_t sendOffset)
    : dataPublisher(baseLogger, inClient, sendEveryX, sendOffset) {
    setToken(apiKey);   // was registrationToken
    setFeedID(feedID);  
    _baseLogger->setSamplingFeatureUUID(feedID);       // was samplingFeatureUUID
}
// Destructor
IoTPlotterPublisher::~IoTPlotterPublisher() {}


void IoTPlotterPublisher::setToken(const char* apiKey) {
    _registrationToken = apiKey;        // was registrationToken
}

void IoTPlotterPublisher::setFeedID(const char* feedID) {
    _feedID = feedID;        // 
}


// Calculates how long the JSON string will be
uint16_t IoTPlotterPublisher::calculateJsonSize() {
    uint16_t jsonLength = 10;  // {"data":{"        start of the JSON string, to be followed by first graph name
    // Cycle through all variables in the _internalArray holding the variable names
    for (uint8_t i = 0; i < _baseLogger->getArrayVarCount(); i++) {
        // Create GRAPH_NAME based on getVarCodeAtI
        jsonLength +=
            _baseLogger->getVarCodeAtI(i).length();  // VarCode length, used as GRAPH_NAME
        
        jsonLength += 12;           //  ":[{"value":   The stuff after GRAPH_NAME, before reporting a numeric value
        jsonLength += _baseLogger->getValueStringAtI(i).length();  // The length of the reported numeric value
        jsonLength += 10;           // , "epoch":
        jsonLength += 10;          // markedLocalEpochTime or markedUTCEpochTime, should be 10 digits
        jsonLength +=  3;           // }]      end of GRAPH_NAME segment

        // Test if there are additional variables to report
        if (i + 1 != _baseLogger->getArrayVarCount()) {
            jsonLength += 2;  // ,"     Start of next GRAPH_NAME iteration
        }
    }
    jsonLength += 2;  // }}   Close off the JSON string

    return jsonLength;
}


// This prints a properly formatted JSON for IoTPlotter to an Arduino stream
void IoTPlotterPublisher::printSensorDataJSON(Stream* stream) {
    stream->print(samplingFeatureTag);     // {"data":{"
    for (uint8_t i = 0; i < _baseLogger->getArrayVarCount(); i++) {
        stream->print(_baseLogger->getVarCodeAtI(i));  // VarCode, used as GRAPH_NAME
        stream->print(JSONvalueTag);     // ":[{"value":
        stream->print(_baseLogger->getValueStringAtI(i));       // print the actual value
        stream->print(epochTag);      // , "epoch":
        stream->print(Logger::markedLocalEpochTime); // print time  options: Logger::markedLocalEpochTime or Logger::markedUTCEpochTime 
        stream->print("}]");

        if (i + 1 != _baseLogger->getArrayVarCount()) { 
            stream->print(",\"");       // start of the next iteration of GRAPH_NAME 
        }
    }

    stream->print("}}");            // close off the JSON string
}


// This prints a fully structured post request for IoTPlotter to the
// specified stream.
void IoTPlotterPublisher::printIoTPlotterRequest(Stream* stream) {
    // Stream the HTTP headers for the post request
    stream->print(postHeader);          // POST  - found in dataPublisherBase.cpp
    stream->print("http://");           // http://                                        
    stream->print(IoTPlotterHost);      // iotplotter.com
    stream->print(postEndpoint);        // /api/v2/feed/
    stream->print(_feedID);             // your feed ID inserted into the http url
    stream->print(HTTPtag);             // HTTP/1.1
    stream->print("\r\nConnection: Close"); // Connection: Close
    stream->print(apiHeader);           // api-key:
    stream->print(_registrationToken);  // the actual API key for your feed
    stream->print(contentTypeHeader);   // "\r\nContent-Type: application/x-www-form-urlencoded"
    stream->print(contentLengthHeader); // "\r\nContent-Length: "
    stream->print(calculateJsonSize()); // Hopefully the correct size of the JSON payload
    stream->print(hostHeader);          // "\r\nHost: "  in dataPublisherBase.cpp
    stream->print(IoTPlotterHost);      // iotplotter.com
    stream->print("\r\n\r\n"); 

    // Stream the JSON itself
    printSensorDataJSON(stream);
}


// A way to begin with everything already set
void IoTPlotterPublisher::begin(Logger& baseLogger, Client* inClient,
                               const char* apiKey,          // was registrationToken
                               const char* feedID) {        // was samplingFeatureUUID
    setToken(apiKey);                                       // was registrationToken
    setFeedID(feedID);
    dataPublisher::begin(baseLogger, inClient);
    _baseLogger->setSamplingFeatureUUID(feedID);            // was samplingFeatureUUID
}
void IoTPlotterPublisher::begin(Logger&     baseLogger,
                               const char* apiKey,          // was registrationToken
                               const char* feedID) {        // was samplingFeatureUUID
    setToken(apiKey);                                       // was registrationToken
    setFeedID(feedID);
    dataPublisher::begin(baseLogger);
    _baseLogger->setSamplingFeatureUUID(feedID);            // was samplingFeatureUUID
}


// This utilizes an attached modem to make a TCP connection to the
// IoTPlotter.com portal and then streams out a post request
// over that connection.
// The return is the http status code of the response.
// int16_t IoTPlotterPublisher::postDataEnviroDIY(void)
int16_t IoTPlotterPublisher::publishData(Client* outClient) {
    // Create a buffer for the portions of the request and response
    char     tempBuffer[37] = "";
    uint16_t did_respond    = 0;

    MS_DBG(F("Outgoing JSON size:"), calculateJsonSize());

    // Open a TCP/IP connection to the IoTPlotter Data Portal 
    MS_DBG(F("Connecting client"));
    MS_START_DEBUG_TIMER;
    if (outClient->connect(IoTPlotterHost, IoTPlotterPort)) {        // TODO: Deal with the port that isn't needed? Or just leave it at 80
        MS_DBG(F("Client connected after"), MS_PRINT_DEBUG_TIMER, F("ms\n"));

        // copy the initial post header into the tx buffer
        snprintf(txBuffer, sizeof(txBuffer), "%s", postHeader);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", "http://");
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", IoTPlotterHost);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", postEndpoint);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", _feedID);                 
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", HTTPtag);

        // add the rest of the HTTP POST headers to the outgoing buffer
        // Before adding each line/chunk to the outgoing buffer, we make sure
        // there is space for that line, sending out buffer if not. This shouldn't
        // really be an issue with the default buffer size of 750 char
        if (bufferFree() < 28) printTxBuffer(outClient);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", "\r\nConnection: Close");

        // Add on the API key header line
        if (bufferFree() < 55) printTxBuffer(outClient);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", apiHeader);  // API key
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", _registrationToken);

        // Add on the content type header line
        if (bufferFree() < 55) printTxBuffer(outClient);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", contentTypeHeader);  // content type

        // Add on the content length header line
        if (bufferFree() < 25) printTxBuffer(outClient);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s",
                 contentLengthHeader);
        itoa(calculateJsonSize(), tempBuffer, 10);  // BASE 10
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", tempBuffer);
        
        // Add on the Host header line
        if (bufferFree() < 25) printTxBuffer(outClient);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", hostHeader);  // Host header
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", IoTPlotterHost);  // Host name 
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", "\r\n");  // newline before JSON package

        // put the start of the JSON into the outgoing response_buffer
        if (bufferFree() < 21) printTxBuffer(outClient);
        snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", samplingFeatureTag);        // sends {\"data\":{\"

            //  For each variable that gets its own graph, you'll need to 
            // start with a \"GRAPH_NAME\":[{ prefix where GRAPH_NAME is the title of your graph
            // \"HALL0\":[{\"value\":xx.xx,\"epoch\":xxxxxxxxxx}]

        for (uint8_t i = 0; i < _baseLogger->getArrayVarCount(); i++) {
            // Once the buffer fills, send it out
            if (bufferFree() < 47) printTxBuffer(outClient);
            // Grab the VarCode, convert to character array, stick in tempBuffer
            _baseLogger->getVarCodeAtI(i).toCharArray(tempBuffer, 37);
            // print tempBuffer - this becomes the GRAPH_NAME on IoTPlotter
            snprintf(txBuffer + strlen(txBuffer),
                     sizeof(txBuffer) - strlen(txBuffer), "%s", tempBuffer);
            // Insert the text that comes before reporting a numeric value                     
            snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", JSONvalueTag);
            // Grab the numeric value for this variable, convert to a character array and stick in tempBuffer
            _baseLogger->getValueStringAtI(i).toCharArray(tempBuffer, 37);
            // Print the variable value that's in tempBuffer now
            snprintf(txBuffer + strlen(txBuffer),
                     sizeof(txBuffer) - strlen(txBuffer), "%s", tempBuffer);
            // Insert the text that comes after a value and before reporting the epoch timestamp                     
            snprintf(txBuffer + strlen(txBuffer),
                 sizeof(txBuffer) - strlen(txBuffer), "%s", epochTag);
            // Get the epoch timestamp and convert to character array to put into tempBuffer
            itoa(Logger::markedLocalEpochTime, tempBuffer, 10);  // BASE 10
            // Print the contents of tempBuffer (the epoch timestamp)
            snprintf(txBuffer + strlen(txBuffer),
                     sizeof(txBuffer) - strlen(txBuffer), "%s", tempBuffer);
            if (i + 1 != _baseLogger->getArrayVarCount()) {     // Prep for another variable to be printed
                txBuffer[strlen(txBuffer)] = '}';
                txBuffer[strlen(txBuffer)] = ']';
                txBuffer[strlen(txBuffer)] = ',';
                txBuffer[strlen(txBuffer)] = '\"';      // to be followed by the next GRAPH_NAME (VarCode)
            } else {        // finish off the JSON string
                txBuffer[strlen(txBuffer)] = '}';
                txBuffer[strlen(txBuffer)] = ']';
                txBuffer[strlen(txBuffer)] = '}';
                txBuffer[strlen(txBuffer)] = '}';
            }
        }

        // Send out the finished request (or the last unsent section of it)
        printTxBuffer(outClient, true);

        // Wait 10 seconds for a response from the server
        uint32_t start = millis();
        while ((millis() - start) < 10000L && outClient->available() < 12) {
            delay(10);
        }

        // Read only the first 12 characters of the response
        // We're only reading as far as the http code, anything beyond that
        // we don't care about.
        did_respond = outClient->readBytes(tempBuffer, 12);

        // Close the TCP/IP connection
        MS_DBG(F("Stopping client"));
        MS_RESET_DEBUG_TIMER;
        outClient->stop();
        MS_DBG(F("Client stopped after"), MS_PRINT_DEBUG_TIMER, F("ms"));
    } else {
        PRINTOUT(F("\n -- Unable to Establish Connection to EnviroDIY Data "
                   "Portal --"));
    }

    // Process the HTTP response
    int16_t responseCode = 0;
    if (did_respond > 0) {
        char responseCode_char[4];
        for (uint8_t i = 0; i < 3; i++) {
            responseCode_char[i] = tempBuffer[i + 9];
        }
        responseCode = atoi(responseCode_char);
    } else {
        responseCode = 504;
    }

    PRINTOUT(F("-- Response Code --"));
    PRINTOUT(responseCode);

    return responseCode;
}
