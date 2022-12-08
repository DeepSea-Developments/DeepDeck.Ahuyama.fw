
#ifndef MQTT_H
#define MQTT_H

void mqtt_app_start(void);

/**
 * @brief Publish a message to a given topic
 * 
 * @param topic topic to be sent
 * @param message message to send
 * @return int -1 if failed, id of message otherwise
 */
int mqtt_pub(char * topic, char * message);

#endif
