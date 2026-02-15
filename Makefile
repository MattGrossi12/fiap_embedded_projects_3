# ThingSpeak MQTT direct helper Makefile (no local broker)
SHELL := /bin/bash

TS_HOST ?= mqtt3.thingspeak.com
TS_PORT ?= 1883

DATA_CHANNEL_ID ?= 3264524
CMD_CHANNEL_ID  ?= 3264562

# Two MQTT Devices to avoid ClientID collision:
# - PC device (used for publish)
PC_ID       ?= DSQABBs2CSsqBT0aPQMsKAU
PC_USER     ?= DSQABBs2CSsqBT0aPQMsKAU
PC_PASS     ?= 1MPwGxAx9IUrj/aIxrtopE8/

# - ESP/read device (used for subscribe/watch by default)
ESP_ID      ?= LjorDzQRMQk9HSYgBzMnDQw
ESP_USER    ?= LjorDzQRMQk9HSYgBzMnDQw
ESP_PASS    ?= qauJy5a6GXxufpJxYhsUcz36

# Select which creds to use: DEVICE=pc or DEVICE=esp
DEVICE ?= pc
ID   := $(if $(filter $(DEVICE),esp),$(ESP_ID),$(PC_ID))
USER := $(if $(filter $(DEVICE),esp),$(ESP_USER),$(PC_USER))
PASS := $(if $(filter $(DEVICE),esp),$(ESP_PASS),$(PC_PASS))

PUB_TOPIC     ?= channels/$(CMD_CHANNEL_ID)/publish
SUB_TOPIC     ?= channels/$(CMD_CHANNEL_ID)/subscribe/#
PUBDATA_TOPIC ?= channels/$(DATA_CHANNEL_ID)/publish
SUBDATA_TOPIC ?= channels/$(DATA_CHANNEL_ID)/subscribe/#

MOSQUITTO_SUB ?= mosquitto_sub
MOSQUITTO_PUB ?= mosquitto_pub

# Force MQTT 3.1.1 (ThingSpeak compatibility)
MQTT_VER ?= mqttv311

# Modular publish (f1..f8) -> builds field1=...&field2=...
f1 ?= 0
f2 ?= 0
f3 ?= 0
f4 ?= 0
f5 ?= 0
f6 ?= 0
f7 ?= 0
f8 ?= 0
clean = $(patsubst %,%,$(strip $(1)))
MSG_AUTO = field1=$(call clean,$(f1))&field2=$(call clean,$(f2))&field3=$(call clean,$(f3))&field4=$(call clean,$(f4))&field5=$(call clean,$(f5))&field6=$(call clean,$(f6))&field7=$(call clean,$(f7))&field8=$(call clean,$(f8))

MSG ?=

.PHONY: pub pubdata watch watchdata whoami

whoami:
	@echo "DEVICE=$(DEVICE) clientId=$(ID)"
	@echo "CMD_CHANNEL_ID=$(CMD_CHANNEL_ID) DATA_CHANNEL_ID=$(DATA_CHANNEL_ID)"

pub:
	@msg='$(if $(MSG),$(MSG),$(MSG_AUTO))'; \
	echo "PUB (ThingSpeak) DEVICE=$(DEVICE) topic='$(PUB_TOPIC)' msg='$$msg'"; \
	$(MOSQUITTO_PUB) -V "$(MQTT_VER)" -h "$(TS_HOST)" -p "$(TS_PORT)" \
	  -i "$(ID)" -u "$(USER)" -P "$(PASS)" \
	  -t "$(PUB_TOPIC)" -m "$$msg"

pubdata:
	@msg='$(if $(MSG),$(MSG),$(MSG),$(MSG_AUTO))'; \
	echo "PUBDATA (ThingSpeak) DEVICE=$(DEVICE) topic='$(PUBDATA_TOPIC)' msg='$$msg'"; \
	$(MOSQUITTO_PUB) -V "$(MQTT_VER)" -h "$(TS_HOST)" -p "$(TS_PORT)" \
	  -i "$(ID)" -u "$(USER)" -P "$(PASS)" \
	  -t "$(PUBDATA_TOPIC)" -m "$$msg"

watch:
	@echo "WATCH (ThingSpeak) DEVICE=$(DEVICE) topic='$(SUB_TOPIC)'"
	@$(MOSQUITTO_SUB) -V "$(MQTT_VER)" -h "$(TS_HOST)" -p "$(TS_PORT)" \
	  -i "$(ID)" -u "$(USER)" -P "$(PASS)" \
	  -t "$(SUB_TOPIC)" -v

watchdata:
	@echo "WATCHDATA (ThingSpeak) DEVICE=$(DEVICE) topic='$(SUBDATA_TOPIC)'"
	@$(MOSQUITTO_SUB) -V "$(MQTT_VER)" -h "$(TS_HOST)" -p "$(TS_PORT)" \
	  -i "$(ID)" -u "$(USER)" -P "$(PASS)" \
	  -t "$(SUBDATA_TOPIC)" -v
