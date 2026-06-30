.PHONY: build upload monitor flash clean ota port help

PORT_FILE := .pio/upload_port

# ============================================
# Основные команды
# ============================================

build:
	@echo "🔨 Сборка..."
	@pio run -e esp32c3

port:
	@echo "📡 Порты:"
	@pio device list | grep -E "^/dev|Description" || true
	@echo ""
	@echo "Введите порт:"
	@read port && echo "$$port" > $(PORT_FILE)

upload: build
	@if [ -f $(PORT_FILE) ]; then \
		PORT=$$(cat $(PORT_FILE)); \
		echo "📤 USB загрузка: $$PORT"; \
		pio run -e esp32c3 -t upload --upload-port "$$PORT"; \
	else \
		echo "📡 Порты:"; \
		pio device list | grep -E "^/dev|Description" || true; \
		echo "Введите порт:"; \
		read PORT; \
		echo "$$PORT" > $(PORT_FILE); \
		pio run -e esp32c3 -t upload --upload-port "$$PORT"; \
	fi

monitor:
	@if [ -f $(PORT_FILE) ]; then \
		pio device monitor --port "$$(cat $(PORT_FILE))"; \
	else \
		pio device monitor; \
	fi

flash: upload monitor

# ============================================
# OTA загрузка
# ============================================

ota: build
	@echo "📡 OTA загрузка на 192.168.1.252..."
	@pio run -e ota -t upload

ota-ip: build
	@echo "Введите IP:"
	@read IP && pio run -e esp32c3 -t upload --upload-port "$$IP"

# ============================================
# Утилиты
# ============================================

clean:
	@pio run -t clean
	@rm -f $(PORT_FILE)

devices:
	@pio device list

help:
	@echo "LED Star Makefile"
	@echo ""
	@echo "USB:"
	@echo "  make flash   - Сборка + USB загрузка + монитор"
	@echo "  make upload  - Сборка + USB загрузка"
	@echo "  make monitor - Монитор"
	@echo ""
	@echo "OTA:"
	@echo "  make ota     - OTA на 192.168.1.252"
	@echo "  make ota-ip  - OTA с вводом IP"
	@echo ""
	@echo "Другое:"
	@echo "  make build   - Только сборка"
	@echo "  make clean   - Очистка"
	@echo "  make devices - Список устройств"

.DEFAULT_GOAL := help
