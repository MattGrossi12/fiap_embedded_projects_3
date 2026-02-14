# =======================================================
#  Makefile MQTT → CSV estruturado (com parsing JSON)
# =======================================================

BROKER      = hd747018.ala.us-east-1.emqxsl.com
PORT        = 8883
USER        = MATHEUS
PASS        = 123
TOPIC       = wokwi/sensores/fiap

MOSQUITTO_SUB = mosquitto_sub
CSV_FILE   = mqtt_log.csv

listen:
	@echo "data-horario,temp,gas,luz" > $(CSV_FILE)
	@echo " Recebendo dados por MQTT e salvando em $(CSV_FILE)..."

	$(MOSQUITTO_SUB) \
		-h $(BROKER) \
		-p $(PORT) \
		--insecure \
		-u $(USER) -P $(PASS) \
		-t $(TOPIC) | while read line; do \
			TS=$$(date +'%Y-%m-%d %H:%M:%S'); \
			TEMP=$$(echo $$line | jq -r '.temperatura'); \
			GAS=$$(echo $$line | jq -r '.gas'); \
			LUZ=$$(echo $$line | jq -r '.luz'); \
			echo "$$TS,$$TEMP,$$GAS,$$LUZ" | tee -a $(CSV_FILE); \
		done

pub:
	$(MOSQUITTO_SUB:.sub=.pub) \
		-h $(BROKER) \
		-p $(PORT) \
		--insecure \
		-u $(USER) -P $(PASS) \
		-t $(TOPIC) \
		-m "$(MSG)"

clean:
	rm -f $(CSV_FILE)
	echo "CSV removido."

