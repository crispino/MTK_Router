all: $(EXEC)

romfs:
ifeq ($(CONFIG_DEFAULTS_RALINK_MT7621), y)
ifeq ($(CONFIG_RALINKAPP_HWQOS), y)
	ln -sf qos_run_hw_mt7621 qos_run
else
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7621 qos_run
endif
endif
endif
ifeq ($(CONFIG_DEFAULTS_RALINK_MT7620), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7620 qos_run
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_MT7628), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW340AA), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW340AA_JS), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW341AA), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW341AA_JS), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW339AA), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW339AA_JS), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW331AA), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZW331AA_JS), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_RALINK_ZOWEE), y)
ifeq ($(CONFIG_RALINKAPP_SWQOS), y)
	ln -sf qos_run_sw_mt7628 qos_run
else
ifeq ($(CONFIG_CUSTOM_QOS), y)
	$(ROMFSINST) /sbin/qos_custom.sh
	$(ROMFSINST) /sbin/qos_init.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_custom.sh
	chmod 775 $(ROMFSDIR)/sbin/qos_init.sh
endif
endif
endif

ifeq ($(CONFIG_CUSTOM_QOS),)
	$(ROMFSINST) /bin/qos_run
endif
