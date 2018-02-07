/*
*    This file is part of PassFilter,
*    Copyright(C) 2018 Edoardo Brunetti.
*
*    PassFilter is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    PassFilter is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with PassFilter. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

//////////////////////////////////////////
// Functions
size_t mrounds(const size_t number, const size_t multiple) {
    return ((number + multiple - 1) / multiple) * multiple;
}