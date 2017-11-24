TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS +=   \
    src \
    tests

linux {
SUBDIRS += tool
}
