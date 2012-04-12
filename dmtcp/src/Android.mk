LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES:=  connectionstate.cpp \
        connection.cpp connectionidentifier.cpp \
        connectionmanager.cpp connectionrewirer.cpp \
        dmtcpmessagetypes.cpp kernelbufferdrainer.cpp \
        nodetable.cpp protectedfds.cpp \
        sockettable.cpp syslogwrappers.cpp \
        uniquepid.cpp \
        dmtcpcoordinatorapi.cpp \
        util_exec.cpp util_gen.cpp util_init.cpp \
        jalibinterface.cpp processinfo.cpp \
        ckptserializer.cpp resource_manager.cpp \
        binderconnection.cpp \

# sysvipc.cpp

LOCAL_MODULE := libdmtcpinternal
#LOCAL_SHARED_LIBRARIES := libjal libdl libstlport
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

#        dmtcpmodule.cpp syscallsreal.c \
#        dmtcpworker.cpp execwrappers.cpp \
#        filewrappers.cpp threadsync.cpp \
#        glibcsystem.cpp pidwrappers.cpp \
#        workerhijack.cpp mtcpinterface.cpp \
#        remexecwrappers.cpp threadwrappers.cpp \
#        dmtcpawareapi.cpp lookup_service.cpp

include $(CLEAR_VARS)
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES := dmtcphijackhelper.c
LOCAL_SHARED_LIBRARIES := libdl
LOCAL_MODULE := libdmtcphijackhelper
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES := dmtcpawareapi.cpp \
        dmtcpworker.cpp threadsync.cpp \
        dmtcpcoordinatorapi.cpp execwrappers.cpp \
        mtcpinterface.cpp signalwrappers.cpp \
        socketwrappers.cpp workerhijack.cpp \
        threadwrappers.cpp \
        miscwrappers.cpp remexecwrappers.cpp \
        glibcsystem.cpp filewrappers.cpp \
        mallocwrappers.cpp \
        dmtcpplugin.cpp \

# Define ANDROID_SMP appropriately.
ifeq ($(TARGET_CPU_SMP),true)
    LOCAL_CFLAGS += -DANDROID_SMP=1
else
    LOCAL_CFLAGS += -DANDROID_SMP=0
endif


LOCAL_STATIC_LIBRARIES := libdmtcpinternal libjal libsyscallsreal
LOCAL_SHARED_LIBRARIES := libdl libstlport libdmtcphijackhelper
LOCAL_MODULE := dmtcphijack
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES := nosyscallsreal.c dmtcpnohijackstubs.cpp

LOCAL_MODULE := libnohijack
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES := dmtcpaware.c

LOCAL_MODULE := libdmtcpaware
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES := dmtcpaware.c

LOCAL_MODULE := libdmtcpaware
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_SRC_FILES := syscallsreal.c trampolines.cpp

LOCAL_MODULE := libsyscallsreal
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_SRC_FILES := dmtcp_coordinator.cpp lookup_service.cpp
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_LDFLAGS:=
LOCAL_MODULE := dmtcp_coordinator
LOCAL_STATIC_LIBRARIES := libdmtcpinternal libjal libnohijack
LOCAL_SHARED_LIBRARIES := libdl libstlport
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_SRC_FILES := dmtcp_checkpoint.cpp
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_LDFLAGS:=
LOCAL_MODULE := dmtcp_checkpoint
LOCAL_STATIC_LIBRARIES := libdmtcpinternal libjal libnohijack
LOCAL_SHARED_LIBRARIES := libdl libstlport
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_SRC_FILES := dmtcp_restart.cpp restoretarget.cpp
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_LDFLAGS:=
LOCAL_MODULE := dmtcp_restart
LOCAL_STATIC_LIBRARIES := libdmtcpinternal libjal libnohijack
LOCAL_SHARED_LIBRARIES := libdl libstlport
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_SRC_FILES := dmtcp_command.cpp
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_LDFLAGS:=
LOCAL_MODULE := dmtcp_command
LOCAL_STATIC_LIBRARIES := libdmtcpinternal libjal libnohijack
LOCAL_SHARED_LIBRARIES := libdl libstlport
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := external/stlport/stlport \
                    bionic/
LOCAL_SRC_FILES := dmtcp_inspector.cpp
LOCAL_CFLAGS+= -O0 -g3 -DHAVE_CONFIG_H
LOCAL_LDFLAGS:=
LOCAL_MODULE := dmtcp_inspector
LOCAL_STATIC_LIBRARIES := libdmtcpinternal libjal libnohijack
LOCAL_SHARED_LIBRARIES := libdl libstlport
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
