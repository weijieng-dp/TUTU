/*!
@file       Tileset.h
@author     Ou Yukang (yukang.ou) 100%
@date       08/01/2026
@brief		Interface for tileset object. Loading and unloading is handled
            by the ResourceManager.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "pch.h"
#include "Tileset.h"
#include "rapidjson/document.h"
#include "ResourceManager.h"
#include "CEO.h"


Tileset::Tileset() : path{ "" }, texture{ &CEO::Get<ResourceManager>()->GetErrorTex()}, sprites{ 

} {}
