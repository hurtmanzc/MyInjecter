#pragma once

BOOL DetourApi(PVOID *ppPointer, PVOID pDetour);
BOOL UnDetourApi(PVOID *ppPointer, PVOID pDetour);

BOOL DoDetour();
BOOL DoUndetour();



