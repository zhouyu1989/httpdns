
include $(TOPDIR)/rules.mk

PKG_NAME:=httpdns
PKG_VERSION:=
PKG_RELEASE:=1

#PKG_SOURCE_PROTO:=git
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=https://github.com/aliyun/aliyun-log-c-sdk.git
#PKG_SOURCE_VERSION:=$(PKG_VERSION)

PKG_SOURCE_DIR:=$(ROKID_DIR)/frameworks/native/libs/httpdns

PKG_FIXUP:=libtool
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk


PRODUCT_NAME=kamino


define Package/httpdns
  SECTION:=libs
  CATEGORY:=rokid
  TITLE:=httpdns
  DEPENDS:=+libstdcpp +curl +cJSON +libpthread
  URL:=
endef

define Package/httpdns/description
	httpdns lib
endef

TARGET_CFLAGS += $(FPIC)

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DSTAGING_DIR=
define Build/Prepare
	$(CP) $(PKG_SOURCE_DIR)/* $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so $(1)/usr/lib/
endef

define Package/httpdns/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so $(1)/usr/lib/
#	$(INSTALL_DIR) $(1)/usr/bin
#	$(CP) $(PKG_INSTALL_DIR)/usr/bin/httpdns_sample $(1)/usr/bin/
endef

$(eval $(call BuildPackage,httpdns))

