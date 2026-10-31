#pragma once
// 将此文件复制为 tls_config.h，并填入服务端证书链对应的真实根CA和HTTPS URL。
// 未配置时 main.cpp 不发起 HTTPS，HTTP/WebSocket 局域网演示仍可用。
#define HTTPS_URL "https://YOUR_DOMAIN/status"
static const char ROOT_CA[] = R"PEM(-----BEGIN CERTIFICATE-----
REPLACE_WITH_REAL_CA_PEM
-----END CERTIFICATE-----
)PEM";
