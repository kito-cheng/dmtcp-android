LOCAL_PATH:= $(call my-dir)

MTCP_LOCAL_CFLAGS:=-DHIGHEST_VA='(VA)0xffffe000' -fno-stack-protector

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= mtcp.c mtcp_restart_nolibc.c \
        mtcp_maybebpt.c mtcp_printf.c mtcp_util.c \
        mtcp_safemmap.c mtcp_safe_open.c \
        mtcp_state.c mtcp_check_vdso.c mtcp_sigaction.c mtcp_fastckpt.c
#        getcontest-x86.S
LOCAL_C_INCLUDES := bionic/libc/private/ \
                    bionic/libc/bionic/

LOCAL_CFLAGS+= $(MTCP_LOCAL_CFLAGS) -fno-pic -DDEBUG -DTIMING
LOCAL_LDFLAGS:= -T $(LOCAL_PATH)/mtcp.t -Wl,-Map,$(LOCAL_PATH)/mtcp.map
LOCAL_MODULE := libmtcp
LOCAL_SHARED_LIBRARIES := libc libdl
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := testmtcp.c
LOCAL_CFLAGS+= -O0 -g3 -DDEBUG -DTIMING
LOCAL_LDFLAGS:= -Wl,--export-dynamic
LOCAL_MODULE := testmtcp
LOCAL_SHARED_LIBRARIES := libmtcp
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
