include(cutelogger.pri)

includes.files += $$PWD/*.h
includes.files += \
    $$PWD/DLog

log_dconfig.files += $$PWD/schema/org.deepin.dtkcore.json
log_dconfig.base = $$PWD/schema
log_dconfig.commonid = true
DCONFIG_META_FILES += log_dconfig
load(dtk_install_dconfig)
