#    Copyright (C) 2011 crul authors,
#    
#    This file is part of crul.
#    
#    Butterfly is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Butterfly is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8)
project(crul C)
#set Butterfly options in cache to ovverride them
SET(BUILD_STATIC_LIB ON CACHE BOOL "Build the static Butterfly lib") 
SET(USE_ICU OFF CACHE BOOL "Use the ICU Library for Unicode")
SET(BUILD_SHARED_LIB OFF CACHE BOOL "Build the shared Butterfly lib")
SET(BUILD_UNITTESTS OFF CACHE BOOL "Build the Unittests (recommented)")
ADD_DEFINITIONS(-DBUTTERFLY_USE_ASCII)
add_subdirectory(Butterfly)
add_subdirectory(src)
	
#ADD_EXECUTABLE(crul_test test)
#TARGET_LINK_LIBRARIES(crul_test crul)
