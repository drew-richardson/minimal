// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2018 Drew Richardson <drewrichardson@gmail.com>

#if defined(_WIN32)
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <mswsock.h>
#endif

int main(void) {
#if defined(_WIN32)
  SOCKET sListenSocket = 0;
  SOCKET sAcceptSocket = 0;
  PVOID lpOutputBuffer = 0;
  DWORD dwReceiveDataLength = 0;
  DWORD dwLocalAddressLength = 0;
  DWORD dwRemoteAddressLength = 0;
  DWORD dwBytesReceived = 0;
  OVERLAPPED overlapped;
  BOOL result = AcceptEx(sListenSocket, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, &dwBytesReceived, &overlapped);
  return result;
#else
  return 0;
#endif
}
