// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once

namespace Hilo
{
    // The IResourceLoader class defines the interface used to access resource strings.
    public interface class IResourceLoader
    {
        Platform::String^ GetString(Platform::String^ value);
    };
}