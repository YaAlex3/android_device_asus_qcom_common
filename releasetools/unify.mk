UNIFY_PATH := device/asus/msm8916-common/releasetools


PRODUCT_COPY_FILES += \
		$(UNIFY_PATH)/install.asus.sh:install/bin/install.asus.sh
$(shell mkdir -p  $(UNIFY_PATH)/../../../../out/target/product/Z010D/install/Z010DD/system)
$(shell cp -rf $(UNIFY_PATH)/system/*  $(UNIFY_PATH)/../../../../out/target/product/Z010D/install/Z010DD/system)
