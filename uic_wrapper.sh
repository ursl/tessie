#!/bin/sh
DYLD_FRAMEWORK_PATH=/opt/local/qt5/qtbase/lib${DYLD_FRAMEWORK_PATH:+:$DYLD_FRAMEWORK_PATH}
export DYLD_FRAMEWORK_PATH
QT_PLUGIN_PATH=/opt/local/qt5/qtbase/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}
export QT_PLUGIN_PATH
exec /opt/local/qt5/qtbase/bin/uic "$@"
