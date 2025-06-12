/*
 * Copyright 2025 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wdmp-c.h>
#include "webcfg_rbus.h"
#include "webcfg_method.h"


bool isRbusMethodName(const char *name) 
{

    if (name == NULL && name[0] == '\0')
    {
        WebcfgError("isRbusMethodName: NULL/Empty input received\n");
        return false;
    }

    size_t len = 0;

    while (len < MAX_BUF_SIZE && name[len] != '\0')
    {
        len++;
    }

    if (len >= MAX_BUF_SIZE)
    {
        WebcfgError("isRbusMethodName: String exceeds maximum allowed length (%d), possibly unterminated\n", MAX_BUF_SIZE);
        return false;
    }

    if (len < 3)
    {
        WebcfgError("isRbusMethodName: String too short to be a valid method name: '%s'\n", name);
        return false;
    }

    if (name[len - 2] != '(' || name[len - 1] != ')')
    {
        WebcfgError("isRbusMethodName: String '%s' does not end with '()'\n", name);
        return false;
    }

    WebcfgInfo("isRbusMethodName: Valid method name detected: '%s'\n", name);
    return true;
}

void handleMethod_rbus(const param_t paramVal[], WDMP_STATUS *retStatus, int *ccspStatus)
{
    WebcfgInfo("Inside handleMethod_rbus\n");
    rbusError_t ret = RBUS_ERROR_BUS_ERROR;
    rbusObject_t inParams = NULL;
    rbusObject_t outParams = NULL;
    rbusValue_t val = NULL;

    *retStatus = WDMP_FAILURE;

    rbusHandle_t rbus_handle = get_global_rbus_handle();

    if (!rbus_handle)
    {
        WebcfgError("handleMethod_rbus failed: rbus_handle is not initialized\n");
        return;
    }

    if (!paramVal || !paramVal[0].name || !paramVal[0].value)
    {
        WebcfgError("handleMethod_rbus: Invalid param input\n");
        *retStatus = WDMP_ERR_INVALID_PARAM;
        return;
    }

    WebcfgInfo("handleMethod_rbus: methodName = %s, value = %s, type = %d\n", 
                paramVal[0].name, paramVal[0].value, paramVal[0].type);

    rbusObject_Init(&inParams, NULL);
    rbusValue_Init(&val);
    rbusValue_SetString(val, paramVal[0].value);
    rbusObject_SetValue(inParams, "encoded_blob", val);

    WebcfgInfo("Calling rbusMethod_Invoke..\n");
    ret = rbusMethod_Invoke(rbus_handle, paramVal[0].name, inParams, &outParams);
    WebcfgInfo("rbusMethod_Invoke returned: %d\n", ret);

    *ccspStatus = mapRbusToCcspStatus(ret);
    *retStatus = mapStatus(*ccspStatus);

    rbusValue_Release(val);
    rbusObject_Release(inParams);
    if (outParams)
    {
        rbusObject_Release(outParams);
    }
    return;
}
