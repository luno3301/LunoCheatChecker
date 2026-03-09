#pragma once

#include <string>

// Возвращает MAC-адрес первого физического адаптера в формате "XX-XX-XX-XX-XX-XX".
// GetAdaptersAddresses не требует прав администратора.
// При ошибке возвращает пустую строку.
std::string getMacAddress();
