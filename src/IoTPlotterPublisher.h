/**
 * @file IoTPlotterPublisher.h
 * @copyright 2017-2022 Stroud Water Research Center
 * Part of the EnviroDIY ModularSensors library for Arduino
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Contains the IoTPlotterPublisher subclass of dataPublisher for
 * publishing data to the IoTPlotter.com data portal at
 * http://iotplotter.com
 */

// Header Guards
#ifndef SRC_PUBLISHERS_IOTPLOTTERPUBLISHER_H_
#define SRC_PUBLISHERS_IOTPLOTTERPUBLISHER_H_

// Debugging Statement
#define MS_IOTPLOTTERPUBLISHER_DEBUG

#ifdef MS_IOTPLOTTERPUBLISHER_DEBUG
#define MS_DEBUGGING_STD "IoTPlotterPublisher"
#endif

// Included Dependencies
#include "ModSensorDebugger.h"
#undef MS_DEBUGGING_STD
#include "dataPublisherBase.h"


// ============================================================================
//  Functions for the IoTPlotter data portal receivers.
// ============================================================================
/**
 * @brief The IoTPlotterPublisher subclass of dataPublisher for publishing data
 * to the IoTPlotter data portal at
 * http://iotplotter.com
 *
 * @ingroup the_publishers
 */
class IoTPlotterPublisher : public dataPublisher {
 public:
    // Constructors
    /**
     * @brief Construct a new IoTPlotter Publisher object with no members set.
     */
    IoTPlotterPublisher();
    /**
     * @brief Construct a new IoTPlotter Publisher object
     *
     * @note If a client is never specified, the publisher will attempt to
     * create and use a client on a LoggerModem instance tied to the attached
     * logger.
     *
     * @param baseLogger The logger supplying the data to be published
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     *
     * @note It is possible (though very unlikey) that using this constructor
     * could cause errors if the compiler attempts to initialize the publisher
     * instance before the logger instance.  If you suspect you are seeing that
     * issue, use the null constructor and a populated begin(...) within your
     * set-up function.
     */
    explicit IoTPlotterPublisher(Logger& baseLogger, uint8_t sendEveryX = 1,
                                uint8_t sendOffset = 0);
    /**
     * @brief Construct a new IoTPlotter Publisher object
     *
     * @param baseLogger The logger supplying the data to be published
     * @param inClient An Arduino client instance to use to print data to.
     * Allows the use of any type of client and multiple clients tied to a
     * single TinyGSM modem instance
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     *
     * @note It is possible (though very unlikey) that using this constructor
     * could cause errors if the compiler attempts to initialize the publisher
     * instance before the logger instance.  If you suspect you are seeing that
     * issue, use the null constructor and a populated begin(...) within your
     * set-up function.
     */
    IoTPlotterPublisher(Logger& baseLogger, Client* inClient,
                       uint8_t sendEveryX = 1, uint8_t sendOffset = 0);
    /**
     * @brief Construct a new IoTPlotter Publisher object
     *
     * @param baseLogger The logger supplying the data to be published
     * @param apiKey The API Key for the site on the
     * IoTPlotter.com data portal. Akin to the EnvironDIY registrationToken on MMW
     * @param feedID The feed ID for the site on the
     * IoTPlotter.com data portal. Akin to samplingFeatureUUID from MMW
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     */
    IoTPlotterPublisher(Logger& baseLogger, const char* apiKey,
                       const char* feedID, uint8_t sendEveryX = 1,
                       uint8_t sendOffset = 0);
    /**
     * @brief Construct a new IoTPlotter Publisher object
     *
     * @param baseLogger The logger supplying the data to be published
     * @param inClient An Arduino client instance to use to print data to.
     * Allows the use of any type of client and multiple clients tied to a
     * single TinyGSM modem instance
     * @param apiKey The API Key for the site on the
     * IoTPlotter.com data portal. Akin to the EnvironDIY registrationToken on MMW
     * @param feedID The feed ID for the site on the
     * IoTPlotter.com data portal. Akin to samplingFeatureUUID from MMW
     * @param sendEveryX Currently unimplemented, intended for future use to
     * enable caching and bulk publishing
     * @param sendOffset Currently unimplemented, intended for future use to
     * enable publishing data at a time slightly delayed from when it is
     * collected
     */
    IoTPlotterPublisher(Logger& baseLogger, Client* inClient,
                       const char* apiKey,
                       const char* feedID, uint8_t sendEveryX = 1,
                       uint8_t sendOffset = 0);
    /**
     * @brief Destroy the IoTPlotter Publisher object
     */
    virtual ~IoTPlotterPublisher();

    // Returns the data destination
    String getEndpoint(void) override {
        return String(IoTPlotterHost);
    }

    // Adds the site API Key 
    /**
     * @brief Set the site API Key
     *
     * @param apiKey The API Key for the site on the
     * IoTPlotter.com data portal. Akin to the EnvironDIY registrationToken on MMW
     */
    void setToken(const char* apiKey);

    // Adds the Feed ID 
    /**
     * @brief Set the IoTPlotter.com Feed ID
     *
     * @param feedID The Feed ID for the site on the
     * IoTPlotter.com data portal. Akin to the EnviroDIY samplingFeatureUUID on MMW
     */
    void setFeedID(const char* feedID);

    /**
     * @brief Calculates how long the outgoing JSON will be
     *
     * @return uint16_t The number of characters in the JSON object.
     */
    uint16_t calculateJsonSize();
    // /**
    //  * @brief Calculates how long the full post request will be, including
    //  * headers
    //  *
    //  * @return uint16_t The length of the full request including HTTP
    //  headers.
    //  */
    // uint16_t calculatePostSize();

    /**
     * @brief This generates a properly formatted JSON for EnviroDIY and prints
     * it to the input Arduino stream object.
     *
     * @param stream The Arduino stream to write out the JSON to.
     */
    void printSensorDataJSON(Stream* stream);

    /**
     * @brief This prints a fully structured post request for IoTPlotter to the specified stream.
     *
     * @param stream The Arduino stream to write out the request to.
     */
    void printIoTPlotterRequest(Stream* stream);

    // A way to begin with everything already set
    /**
     * @copydoc dataPublisher::begin(Logger& baseLogger, Client* inClient)
     * @param apiKey The API Key for the site on the
     * IoTPlotter.com data portal. Akin to the EnvironDIY registrationToken on MMW
     * @param feedID The feed ID for the site on the
     * IoTPlotter.com data portal. Akin to samplingFeatureUUID from MMW
     */
    void begin(Logger& baseLogger, Client* inClient,
               const char* apiKey, const char* feedID);
    /**
     * @copydoc dataPublisher::begin(Logger& baseLogger)
     * @param apiKey The API Key for the site on the
     * IoTPlotter.com data portal. Akin to the EnvironDIY registrationToken on MMW
     * @param feedID The feed ID for the site on the
     * IoTPlotter.com data portal. Akin to samplingFeatureUUID from MMW
     */
    void begin(Logger& baseLogger, const char* apiKey,
               const char* feedID);

    // int16_t postDataEnviroDIY(void);
    /**
     * @brief Utilize an attached modem to open a a TCP connection to the
     * EnviroDIY/ODM2DataSharingPortal and then stream out a post request over
     * that connection.
     *
     * This depends on an internet connection already having been made and a
     * client being available.
     *
     * @param outClient An Arduino client instance to use to print data to.
     * Allows the use of any type of client and multiple clients tied to a
     * single TinyGSM modem instance
     * @return **int16_t** The http status code of the response.
     */
    int16_t publishData(Client* outClient) override;

 protected:
    /**
     * @anchor envirodiy_post_vars
     * @name Portions of the POST request to IoTPlotter.com
     *
     * @{
     */
    static const char* postEndpoint;   ///< The endpoint
    static const char* IoTPlotterHost;  ///< The host name
    static const int   IoTPlotterPort;  ///< The host port // LPM: Not needed? 
    static const char* apiHeader;    ///< The token header text
    // static const char *cacheHeader;  ///< The cache header text
    // static const char *connectionHeader;  ///< The keep alive header text
    static const char* contentLengthHeader;  ///< The content length header text
    static const char* contentTypeHeader;    ///< The content type header text
    /**@}*/

    /**
     * @anchor envirodiy_json_vars
     * @name Portions of the JSON object for IoTPlotter.com
     *
     * @{
     */
    static const char* samplingFeatureTag;  ///< The JSON feature UUID tag
    static const char* JSONvalueTag;         //  ":[{\"value\":"     used at start of reporting a value
    static const char* epochTag;             //  ", \"epoch\":"  The JSON feature epoch timestamp tag
                                            /**@}*/

 private:
    // Tokens and UUID's for EnviroDIY
    const char* _registrationToken = nullptr;         
    const char* _feedID = nullptr;         
};

#endif  // SRC_PUBLISHERS_IOTPLOTTERPUBLISHER_H_
