#ifndef LISTENER_H
#define LISTENER_H

#include <QDebug>

#include "FitSDK/fit_decode.hpp"
#include "FitSDK/fit_mesg_broadcaster.hpp"
#include "FitSDK/fit_developer_field_description.hpp"

#include "core/defines.h"
#include "core/types.h"

class Listener
    : public fit::FileIdMesgListener
    , public fit::UserProfileMesgListener
    , public fit::MonitoringMesgListener
    , public fit::DeviceInfoMesgListener
    , public fit::MesgListener
    , public fit::DeveloperFieldDescriptionListener
    , public fit::RecordMesgListener
{
public:
    void SetFitMessageCallback(FitMessageCallback cb) { fitMessage_cb = cb; }

    void OnMesg(fit::Mesg& mesg) override
    {
        printf("On Mesg:\n");
        qDebug() << L"   New Mesg: " << mesg.GetName().c_str() << L".  It has " << mesg.GetNumFields() << L" field(s) and " << mesg.GetNumDevFields() << " developer field(s).\n";

        for (FIT_UINT16 i = 0; i < (FIT_UINT16)mesg.GetNumFields(); i++) {
            Canon::FitMessage fitMsg;

            fit::Field* field = mesg.GetFieldByIndex(i);
            GetFitMsg(fitMsg, field);

            qDebug() << L"   Field" << i << " (" << field->GetName().c_str() << ") has " << field->GetNumValues() << L" value(s)\n";
            PrintValues(*field);
            if (!fitMsg.isValid) {
                qWarning() << "fitMsg is invalid!";
                continue;
            }
            if (fitMessage_cb) { fitMessage_cb(fitMsg); }
            else { qErrnoWarning("fit message is nil!"); }
        }

        for (auto devField : mesg.GetDeveloperFields()) {
            qDebug() << L"   Developer Field(" << devField.GetName().c_str() << ") has " << devField.GetNumValues() << L" value(s)\n";
            PrintValues(devField);
        }
    }

    void OnMesg(fit::FileIdMesg& mesg) override
    {
      printf("File ID:\n");
      if (mesg.IsTypeValid())
         printf("   Type: %d\n", mesg.GetType());
      if (mesg.IsManufacturerValid())
         printf("   Manufacturer: %d\n", mesg.GetManufacturer());
      if (mesg.IsProductValid())
         printf("   Product: %d\n", mesg.GetProduct());
      if (mesg.IsSerialNumberValid())
         printf("   Serial Number: %u\n", mesg.GetSerialNumber());
      if (mesg.IsNumberValid())
         printf("   Number: %d\n", mesg.GetNumber());
    }

    void OnMesg(fit::UserProfileMesg& mesg) override
    {
      printf("User profile:\n");
      if (mesg.IsFriendlyNameValid())
         qDebug() << L"   Friendly Name: " << mesg.GetFriendlyName().c_str() << L"\n";
      if (mesg.GetGender() == FIT_GENDER_MALE)
         printf("   Gender: Male\n");
      if (mesg.GetGender() == FIT_GENDER_FEMALE)
         printf("   Gender: Female\n");
      if (mesg.IsAgeValid())
         printf("   Age [years]: %d\n", mesg.GetAge());
      if ( mesg.IsWeightValid() )
         printf("   Weight [kg]: %0.2f\n", mesg.GetWeight());
    }

    void OnMesg(fit::DeviceInfoMesg& mesg) override
    {
      printf("Device info:\n");

      if (mesg.IsTimestampValid())
         printf("   Timestamp: %d\n", mesg.GetTimestamp());

      switch(mesg.GetBatteryStatus())
      {
      case FIT_BATTERY_STATUS_CRITICAL:
         printf("   Battery status: Critical\n");
         break;
      case FIT_BATTERY_STATUS_GOOD:
         printf("   Battery status: Good\n");
         break;
      case FIT_BATTERY_STATUS_LOW:
         printf("   Battery status: Low\n");
         break;
      case FIT_BATTERY_STATUS_NEW:
         printf("   Battery status: New\n");
         break;
      case FIT_BATTERY_STATUS_OK:
         printf("   Battery status: OK\n");
         break;
      default:
         printf("   Battery status: Invalid\n");
         break;
      }
    }

    void OnMesg(fit::MonitoringMesg& mesg) override
    {
      printf("Monitoring:\n");

      if (mesg.IsTimestampValid())
      {
         printf("   Timestamp: %d\n", mesg.GetTimestamp());
      }

      if(mesg.IsActivityTypeValid())
      {
         printf("   Activity type: %d\n", mesg.GetActivityType());
      }

      switch(mesg.GetActivityType()) // The Cycling field is dynamic
      {
      case FIT_ACTIVITY_TYPE_WALKING:
      case FIT_ACTIVITY_TYPE_RUNNING: // Intentional fallthrough
         if(mesg.IsStepsValid())
         {
            printf("   Steps: %d\n", mesg.GetSteps());
         }
         break;
      case FIT_ACTIVITY_TYPE_CYCLING:
      case FIT_ACTIVITY_TYPE_SWIMMING: // Intentional fallthrough
         if( mesg.IsStrokesValid() )
         {
            printf(   "Strokes: %f\n", mesg.GetStrokes());
         }
         break;
      default:
         if(mesg.IsCyclesValid() )
         {
            printf(   "Cycles: %f\n", mesg.GetCycles());
         }
         break;
      }
    }

    void OnMesg( fit::RecordMesg& record ) override
    {
        printf( "Record:\n" );
        PrintOverrideValues( record, fit::RecordMesg::FieldDefNum::HeartRate);
        PrintOverrideValues( record, fit::RecordMesg::FieldDefNum::Cadence );
        PrintOverrideValues( record, fit::RecordMesg::FieldDefNum::Distance );
        PrintOverrideValues( record, fit::RecordMesg::FieldDefNum::Speed );
    }

    void OnDeveloperFieldDescription( const fit::DeveloperFieldDescription& desc ) override
    {
       printf( "New Developer Field Description\n" );
       printf( "   App Version: %d\n", desc.GetApplicationVersion() );
       printf( "   Field Number: %d\n", desc.GetFieldDefinitionNumber() );
    }

    void GetFitMsg(Canon::FitMessage& fitMsg, const fit::FieldBase* field) {
        const auto& name  = field->GetName();
        for (FIT_UINT8 j = 0; j < (FIT_UINT8)field->GetNumValues(); ++j) {
            fitMsg.isValid = true;
            switch (field->GetType()) {
                case FIT_BASE_TYPE_ENUM:
                case FIT_BASE_TYPE_BYTE:
                case FIT_BASE_TYPE_SINT8:
                case FIT_BASE_TYPE_UINT8:
                case FIT_BASE_TYPE_SINT16:
                case FIT_BASE_TYPE_UINT16:
                case FIT_BASE_TYPE_SINT32:
                case FIT_BASE_TYPE_UINT32:
                case FIT_BASE_TYPE_SINT64:
                case FIT_BASE_TYPE_UINT64:
                case FIT_BASE_TYPE_UINT8Z:
                case FIT_BASE_TYPE_UINT16Z:
                case FIT_BASE_TYPE_UINT32Z:
                case FIT_BASE_TYPE_UINT64Z:
                case FIT_BASE_TYPE_FLOAT32:
                case FIT_BASE_TYPE_FLOAT64:
                    FIT_FLOAT64 ret = field->GetFLOAT64Value(j);
                    if      (name == "timestamp")           { fitMsg.timeStamp     = static_cast<uint64_t>(ret); }
                    else if (name == "speed")               { fitMsg.speed         = static_cast<uint16_t>(ret); }
                    else if (name == "temperature")         { fitMsg.temperature   = static_cast<uint16_t>(ret); }
                    else if (name == "distance")            { fitMsg.distance      = static_cast<uint32_t>(ret); }
                    else if (name == "grade")               { fitMsg.grade         = ret;                        }
                    else if (name == "position_lat")        { fitMsg.position_lat  = ret;                        }
                    else if (name == "position_long")       { fitMsg.position_long = ret;                        }
                    else if (name == "enhanced_speed")      {}
                    else if (name == "enhanced_altitude")   {}
                    else if (name == "altitude")            {}
                    break;
            }
        }


    }

    static void PrintValues(const fit::FieldBase& field)
    {
       for (FIT_UINT8 j=0; j< (FIT_UINT8)field.GetNumValues(); j++)
       {
           qDebug() << L"       Val" << j << L": ";
           switch (field.GetType())
           {
           // Get float 64 values for numeric types to receive values that have
           // their scale and offset properly applied.
               case FIT_BASE_TYPE_ENUM:
               case FIT_BASE_TYPE_BYTE:
               case FIT_BASE_TYPE_SINT8:
               case FIT_BASE_TYPE_UINT8:
               case FIT_BASE_TYPE_SINT16:
               case FIT_BASE_TYPE_UINT16:
               case FIT_BASE_TYPE_SINT32:
               case FIT_BASE_TYPE_UINT32:
               case FIT_BASE_TYPE_SINT64:
               case FIT_BASE_TYPE_UINT64:
               case FIT_BASE_TYPE_UINT8Z:
               case FIT_BASE_TYPE_UINT16Z:
               case FIT_BASE_TYPE_UINT32Z:
               case FIT_BASE_TYPE_UINT64Z:
               case FIT_BASE_TYPE_FLOAT32:
               case FIT_BASE_TYPE_FLOAT64:
                   qDebug() << field.GetFLOAT64Value(j);
                   break;
               case FIT_BASE_TYPE_STRING:
                   qDebug() << field.GetSTRINGValue(j).c_str();
                   break;
               default:
                   break;
           }
           qDebug() << L" " << field.GetUnits().c_str() << L"\n";;
       }
    }

    static void PrintOverrideValues( const fit::Mesg& mesg, FIT_UINT8 fieldNum )
    {
       std::vector<const fit::FieldBase*> fields = mesg.GetOverrideFields( fieldNum );
       const fit::Profile::FIELD * profileField = fit::Profile::GetField( mesg.GetNum(), fieldNum );
       FIT_BOOL namePrinted = FIT_FALSE;

       for ( const fit::FieldBase* field : fields )
       {
           if ( !namePrinted )
           {
               printf( "   %s:\n", profileField->name.c_str() );
               namePrinted = FIT_TRUE;
           }

           if ( FIT_NULL != dynamic_cast<const fit::Field*>( field ) )
           {
               // Native Field
               printf( "      native: " );
           }
           else
           {
               // Developer Field
               printf( "      override: " );
           }

           switch (field->GetType())
           {
           // Get float 64 values for numeric types to receive values that have
           // their scale and offset properly applied.
           case FIT_BASE_TYPE_ENUM:
           case FIT_BASE_TYPE_BYTE:
           case FIT_BASE_TYPE_SINT8:
           case FIT_BASE_TYPE_UINT8:
           case FIT_BASE_TYPE_SINT16:
           case FIT_BASE_TYPE_UINT16:
           case FIT_BASE_TYPE_SINT32:
           case FIT_BASE_TYPE_UINT32:
           case FIT_BASE_TYPE_SINT64:
           case FIT_BASE_TYPE_UINT64:
           case FIT_BASE_TYPE_UINT8Z:
           case FIT_BASE_TYPE_UINT16Z:
           case FIT_BASE_TYPE_UINT32Z:
           case FIT_BASE_TYPE_UINT64Z:
           case FIT_BASE_TYPE_FLOAT32:
           case FIT_BASE_TYPE_FLOAT64:
               printf("%f\n", field->GetFLOAT64Value());
               break;
           case FIT_BASE_TYPE_STRING:
               printf("%ls\n", field->GetSTRINGValue().c_str());
               break;
           default:
               break;
           }
       }
    }

private:
    FitMessageCallback fitMessage_cb;
};

#endif // LISTENER_H
