LOCAL_PATH := device/asus/msm8916-common/releasetools


PRODUCT_COPY_FILES += \
		$(LOCAL_PATH)/install.asus.sh:install/bin/install.asus.sh
$(shell mkdir -p  $(LOCAL_PATH)/../../../../out/target/product/Z010D/install/Z010DD/system)
$(shell cp -rf $(LOCAL_PATH)/system/*  $(LOCAL_PATH)/../../../../out/target/product/Z010D/install/Z010DD/system)
