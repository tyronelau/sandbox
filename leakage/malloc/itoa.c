/* Internal function for converting integers to ASCII.
   Copyright (C) 1994-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Torbjorn Granlund <tege@matematik.su.se>
   and Ulrich Drepper <drepper@gnu.org>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

static const char s_itoa_upper_digits[36] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char s_itoa_lower_digits[36] = "0123456789abcdefghijklmnopqrstuvwxyz";

char * _itoa_word(unsigned long value, char *buflim,
	    unsigned int base, int upper_case) {
  const char *digits = (upper_case
			? s_itoa_upper_digits
			: s_itoa_lower_digits);

  switch (base)
    {
#define SPECIAL(Base)							      \
    case Base:								      \
      do								      \
	*--buflim = digits[value % Base];				      \
      while ((value /= Base) != 0);					      \
      break

      SPECIAL (10);
      SPECIAL (16);
      SPECIAL (8);
    default:
      do
	*--buflim = digits[value % base];
      while ((value /= base) != 0);
    }
  return buflim;
}

