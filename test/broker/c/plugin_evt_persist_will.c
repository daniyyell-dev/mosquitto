#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <mosquitto/broker.h>
#include <mosquitto/broker_plugin.h>

MOSQUITTO_PLUGIN_DECLARE_VERSION(5);

static mosquitto_plugin_id_t *plg_id;
static int add_event = MOSQ_EVT_PERSIST_WILL_ADD;
static int delete_event = MOSQ_EVT_PERSIST_WILL_DELETE;

static int callback_persist_will(int event, void *event_data, void *user_data)
{
	struct mosquitto_evt_persist_will_msg *ed = event_data;
	int expected_event = *(int *)user_data;
	const char *topic;

	if(strcmp(ed->data.clientid, "will-client")){
		return MOSQ_ERR_SUCCESS;
	}
	if(event != expected_event){
		abort();
	}
	topic = event == MOSQ_EVT_PERSIST_WILL_ADD ? "evt/will/add" : "evt/will/delete";
	mosquitto_broker_publish_copy(NULL, topic, (int)strlen(ed->data.clientid), ed->data.clientid, 0, false, NULL);
	return MOSQ_ERR_SUCCESS;
}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	(void)user_data;
	(void)opts;
	(void)opt_count;
	plg_id = identifier;
	mosquitto_callback_register(plg_id, MOSQ_EVT_PERSIST_WILL_ADD, callback_persist_will, NULL, &add_event);
	mosquitto_callback_register(plg_id, MOSQ_EVT_PERSIST_WILL_DELETE, callback_persist_will, NULL, &delete_event);
	return MOSQ_ERR_SUCCESS;
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	(void)user_data;
	(void)opts;
	(void)opt_count;
	mosquitto_callback_unregister(plg_id, MOSQ_EVT_PERSIST_WILL_ADD, callback_persist_will, NULL);
	mosquitto_callback_unregister(plg_id, MOSQ_EVT_PERSIST_WILL_DELETE, callback_persist_will, NULL);
	return MOSQ_ERR_SUCCESS;
}
