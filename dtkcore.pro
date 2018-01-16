TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS +=   \
    src \
    tests

!mac:!win*: SUBDIRS += tool
