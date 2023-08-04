/**
 * \file Version.h
 * \brief Version and Build NumberHelper Class
 *
 * This helper macros exposes the static methods to get the firmware version and the build number.
 * Use the build() and version() metho5ds anywhere in the program including this file
*/
#ifndef __VERSION_H__
#define __VERSION_H__

//! Incremental build number
#define build() "1.0.4 rc"
//! Firmware version
#define version() "1.1"
//! Project name
#define project() "Electroschematics"

#endif //__VERSION_H__
