/*
 * Copyright (C) 2015, Christian Benjamin Ries
 * Website: http://www.christianbenjaminries.de
 * License: MIT License, http://opensource.org/licenses/MIT
 */

#ifndef __SEGENERAL_H__
#define __SEGENERAL_H__

#pragma once

#define SE_CONT4NULL(ptr) if(ptr == NULL) { continue; }
#define SE_CONT4TRUE(res) if(res == true) { continue; }
#define SE_DELETE(ptr) if(ptr != NULL) { delete ptr; ptr = NULL; }

#endif // __SEGENERAL_H__

