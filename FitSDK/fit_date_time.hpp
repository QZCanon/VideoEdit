/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2024 Garmin International, Inc.
// Licensed under the Flexible and Interoperable Data Transfer (FIT) Protocol License; you
// may not use this file except in compliance with the Flexible and Interoperable Data
// Transfer (FIT) Protocol License.
/////////////////////////////////////////////////////////////////////////////////////////////
// ****WARNING****  This file is auto-generated!  Do NOT edit this file.
// Profile Version = 21.141.0Release
// Tag = production/release/21.141.0-0-g2aa27e1
/////////////////////////////////////////////////////////////////////////////////////////////


#if !defined(FIT_DATE_TIME_HPP)
#define FIT_DATE_TIME_HPP

#include <ctime>
#include <cmath>
#include "fit_profile.hpp"
namespace fit
{

class DateTime
{
   public:
      DateTime(time_t timeStamp);
      DateTime(FIT_DATE_TIME timeStamp);
      DateTime(FIT_DATE_TIME timeStamp, FIT_FLOAT64 fractionalTimeStamp);
      DateTime(const DateTime& dateTime);
      bool Equals(DateTime dateTime);
      FIT_DATE_TIME GetTimeStamp();
      FIT_FLOAT64 GetFractionalTimestamp();
      void add(DateTime dateTime);
      void add(FIT_DATE_TIME timestamp);
      void add(double fractional_timestamp);
      time_t GetTimeT();
      void ConvertSystemTimeToUTC(long offset);
      int CompareTo(DateTime t1);

   private:
      FIT_DATE_TIME timeStamp;
      FIT_FLOAT64 fractionalTimeStamp;
      static const time_t systemTimeOffset = 631065600; // Needed for conversion from UNIX time to FIT time
};

} // namespace fit

#endif // !defined(FIT_DATE_TIME_HPP)
