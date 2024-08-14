QMAKE_MACOSX_DEPLOYMENT_TARGET=10.15
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG+=sdk_no_version_check


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FitParse/fitparse.cpp \
    FitSDK/fit.cpp \
    FitSDK/fit_accumulated_field.cpp \
    FitSDK/fit_accumulator.cpp \
    FitSDK/fit_buffer_encode.cpp \
    FitSDK/fit_buffered_mesg_broadcaster.cpp \
    FitSDK/fit_buffered_record_mesg_broadcaster.cpp \
    FitSDK/fit_crc.cpp \
    FitSDK/fit_date_time.cpp \
    FitSDK/fit_decode.cpp \
    FitSDK/fit_developer_field.cpp \
    FitSDK/fit_developer_field_definition.cpp \
    FitSDK/fit_developer_field_description.cpp \
    FitSDK/fit_encode.cpp \
    FitSDK/fit_factory.cpp \
    FitSDK/fit_field.cpp \
    FitSDK/fit_field_base.cpp \
    FitSDK/fit_field_definition.cpp \
    FitSDK/fit_mesg.cpp \
    FitSDK/fit_mesg_broadcaster.cpp \
    FitSDK/fit_mesg_definition.cpp \
    FitSDK/fit_mesg_with_event_broadcaster.cpp \
    FitSDK/fit_profile.cpp \
    FitSDK/fit_protocol_validator.cpp \
    FitSDK/fit_unicode.cpp \
    dashboard/dashboard.cpp \
    main.cpp \
    mainwindow.cpp \
    myslide/myslide.cpp

HEADERS += \
    FitParse/fitparse.h \
    FitParse/listener.h \
    FitSDK/fit.hpp \
    FitSDK/fit_aad_accel_features_mesg.hpp \
    FitSDK/fit_aad_accel_features_mesg_listener.hpp \
    FitSDK/fit_accelerometer_data_mesg.hpp \
    FitSDK/fit_accelerometer_data_mesg_listener.hpp \
    FitSDK/fit_accumulated_field.hpp \
    FitSDK/fit_accumulator.hpp \
    FitSDK/fit_activity_mesg.hpp \
    FitSDK/fit_activity_mesg_listener.hpp \
    FitSDK/fit_ant_channel_id_mesg.hpp \
    FitSDK/fit_ant_channel_id_mesg_listener.hpp \
    FitSDK/fit_ant_rx_mesg.hpp \
    FitSDK/fit_ant_rx_mesg_listener.hpp \
    FitSDK/fit_ant_tx_mesg.hpp \
    FitSDK/fit_ant_tx_mesg_listener.hpp \
    FitSDK/fit_aviation_attitude_mesg.hpp \
    FitSDK/fit_aviation_attitude_mesg_listener.hpp \
    FitSDK/fit_barometer_data_mesg.hpp \
    FitSDK/fit_barometer_data_mesg_listener.hpp \
    FitSDK/fit_beat_intervals_mesg.hpp \
    FitSDK/fit_beat_intervals_mesg_listener.hpp \
    FitSDK/fit_bike_profile_mesg.hpp \
    FitSDK/fit_bike_profile_mesg_listener.hpp \
    FitSDK/fit_blood_pressure_mesg.hpp \
    FitSDK/fit_blood_pressure_mesg_listener.hpp \
    FitSDK/fit_buffer_encode.hpp \
    FitSDK/fit_buffered_mesg_broadcaster.hpp \
    FitSDK/fit_buffered_record_mesg.hpp \
    FitSDK/fit_buffered_record_mesg_broadcaster.hpp \
    FitSDK/fit_buffered_record_mesg_listener.hpp \
    FitSDK/fit_cadence_zone_mesg.hpp \
    FitSDK/fit_cadence_zone_mesg_listener.hpp \
    FitSDK/fit_camera_event_mesg.hpp \
    FitSDK/fit_camera_event_mesg_listener.hpp \
    FitSDK/fit_capabilities_mesg.hpp \
    FitSDK/fit_capabilities_mesg_listener.hpp \
    FitSDK/fit_chrono_shot_data_mesg.hpp \
    FitSDK/fit_chrono_shot_data_mesg_listener.hpp \
    FitSDK/fit_chrono_shot_session_mesg.hpp \
    FitSDK/fit_chrono_shot_session_mesg_listener.hpp \
    FitSDK/fit_climb_pro_mesg.hpp \
    FitSDK/fit_climb_pro_mesg_listener.hpp \
    FitSDK/fit_config.hpp \
    FitSDK/fit_connectivity_mesg.hpp \
    FitSDK/fit_connectivity_mesg_listener.hpp \
    FitSDK/fit_course_mesg.hpp \
    FitSDK/fit_course_mesg_listener.hpp \
    FitSDK/fit_course_point_mesg.hpp \
    FitSDK/fit_course_point_mesg_listener.hpp \
    FitSDK/fit_crc.hpp \
    FitSDK/fit_date_time.hpp \
    FitSDK/fit_decode.hpp \
    FitSDK/fit_developer_data_id_mesg.hpp \
    FitSDK/fit_developer_data_id_mesg_listener.hpp \
    FitSDK/fit_developer_field.hpp \
    FitSDK/fit_developer_field_definition.hpp \
    FitSDK/fit_developer_field_description.hpp \
    FitSDK/fit_developer_field_description_listener.hpp \
    FitSDK/fit_device_aux_battery_info_mesg.hpp \
    FitSDK/fit_device_aux_battery_info_mesg_listener.hpp \
    FitSDK/fit_device_info_mesg.hpp \
    FitSDK/fit_device_info_mesg_listener.hpp \
    FitSDK/fit_device_settings_mesg.hpp \
    FitSDK/fit_device_settings_mesg_listener.hpp \
    FitSDK/fit_dive_alarm_mesg.hpp \
    FitSDK/fit_dive_alarm_mesg_listener.hpp \
    FitSDK/fit_dive_apnea_alarm_mesg.hpp \
    FitSDK/fit_dive_apnea_alarm_mesg_listener.hpp \
    FitSDK/fit_dive_gas_mesg.hpp \
    FitSDK/fit_dive_gas_mesg_listener.hpp \
    FitSDK/fit_dive_settings_mesg.hpp \
    FitSDK/fit_dive_settings_mesg_listener.hpp \
    FitSDK/fit_dive_summary_mesg.hpp \
    FitSDK/fit_dive_summary_mesg_listener.hpp \
    FitSDK/fit_encode.hpp \
    FitSDK/fit_event_mesg.hpp \
    FitSDK/fit_event_mesg_listener.hpp \
    FitSDK/fit_exd_data_concept_configuration_mesg.hpp \
    FitSDK/fit_exd_data_concept_configuration_mesg_listener.hpp \
    FitSDK/fit_exd_data_field_configuration_mesg.hpp \
    FitSDK/fit_exd_data_field_configuration_mesg_listener.hpp \
    FitSDK/fit_exd_screen_configuration_mesg.hpp \
    FitSDK/fit_exd_screen_configuration_mesg_listener.hpp \
    FitSDK/fit_exercise_title_mesg.hpp \
    FitSDK/fit_exercise_title_mesg_listener.hpp \
    FitSDK/fit_factory.hpp \
    FitSDK/fit_field.hpp \
    FitSDK/fit_field_base.hpp \
    FitSDK/fit_field_capabilities_mesg.hpp \
    FitSDK/fit_field_capabilities_mesg_listener.hpp \
    FitSDK/fit_field_definition.hpp \
    FitSDK/fit_field_description_mesg.hpp \
    FitSDK/fit_field_description_mesg_listener.hpp \
    FitSDK/fit_file_capabilities_mesg.hpp \
    FitSDK/fit_file_capabilities_mesg_listener.hpp \
    FitSDK/fit_file_creator_mesg.hpp \
    FitSDK/fit_file_creator_mesg_listener.hpp \
    FitSDK/fit_file_id_mesg.hpp \
    FitSDK/fit_file_id_mesg_listener.hpp \
    FitSDK/fit_goal_mesg.hpp \
    FitSDK/fit_goal_mesg_listener.hpp \
    FitSDK/fit_gps_metadata_mesg.hpp \
    FitSDK/fit_gps_metadata_mesg_listener.hpp \
    FitSDK/fit_gyroscope_data_mesg.hpp \
    FitSDK/fit_gyroscope_data_mesg_listener.hpp \
    FitSDK/fit_hr_mesg.hpp \
    FitSDK/fit_hr_mesg_listener.hpp \
    FitSDK/fit_hr_zone_mesg.hpp \
    FitSDK/fit_hr_zone_mesg_listener.hpp \
    FitSDK/fit_hrm_profile_mesg.hpp \
    FitSDK/fit_hrm_profile_mesg_listener.hpp \
    FitSDK/fit_hrv_mesg.hpp \
    FitSDK/fit_hrv_mesg_listener.hpp \
    FitSDK/fit_hrv_status_summary_mesg.hpp \
    FitSDK/fit_hrv_status_summary_mesg_listener.hpp \
    FitSDK/fit_hrv_value_mesg.hpp \
    FitSDK/fit_hrv_value_mesg_listener.hpp \
    FitSDK/fit_hsa_accelerometer_data_mesg.hpp \
    FitSDK/fit_hsa_accelerometer_data_mesg_listener.hpp \
    FitSDK/fit_hsa_body_battery_data_mesg.hpp \
    FitSDK/fit_hsa_body_battery_data_mesg_listener.hpp \
    FitSDK/fit_hsa_configuration_data_mesg.hpp \
    FitSDK/fit_hsa_configuration_data_mesg_listener.hpp \
    FitSDK/fit_hsa_event_mesg.hpp \
    FitSDK/fit_hsa_event_mesg_listener.hpp \
    FitSDK/fit_hsa_gyroscope_data_mesg.hpp \
    FitSDK/fit_hsa_gyroscope_data_mesg_listener.hpp \
    FitSDK/fit_hsa_heart_rate_data_mesg.hpp \
    FitSDK/fit_hsa_heart_rate_data_mesg_listener.hpp \
    FitSDK/fit_hsa_respiration_data_mesg.hpp \
    FitSDK/fit_hsa_respiration_data_mesg_listener.hpp \
    FitSDK/fit_hsa_spo2_data_mesg.hpp \
    FitSDK/fit_hsa_spo2_data_mesg_listener.hpp \
    FitSDK/fit_hsa_step_data_mesg.hpp \
    FitSDK/fit_hsa_step_data_mesg_listener.hpp \
    FitSDK/fit_hsa_stress_data_mesg.hpp \
    FitSDK/fit_hsa_stress_data_mesg_listener.hpp \
    FitSDK/fit_hsa_wrist_temperature_data_mesg.hpp \
    FitSDK/fit_hsa_wrist_temperature_data_mesg_listener.hpp \
    FitSDK/fit_jump_mesg.hpp \
    FitSDK/fit_jump_mesg_listener.hpp \
    FitSDK/fit_lap_mesg.hpp \
    FitSDK/fit_lap_mesg_listener.hpp \
    FitSDK/fit_length_mesg.hpp \
    FitSDK/fit_length_mesg_listener.hpp \
    FitSDK/fit_magnetometer_data_mesg.hpp \
    FitSDK/fit_magnetometer_data_mesg_listener.hpp \
    FitSDK/fit_max_met_data_mesg.hpp \
    FitSDK/fit_max_met_data_mesg_listener.hpp \
    FitSDK/fit_memo_glob_mesg.hpp \
    FitSDK/fit_memo_glob_mesg_listener.hpp \
    FitSDK/fit_mesg.hpp \
    FitSDK/fit_mesg_broadcast_plugin.hpp \
    FitSDK/fit_mesg_broadcaster.hpp \
    FitSDK/fit_mesg_capabilities_mesg.hpp \
    FitSDK/fit_mesg_capabilities_mesg_listener.hpp \
    FitSDK/fit_mesg_definition.hpp \
    FitSDK/fit_mesg_definition_listener.hpp \
    FitSDK/fit_mesg_listener.hpp \
    FitSDK/fit_mesg_with_event.hpp \
    FitSDK/fit_mesg_with_event_broadcaster.hpp \
    FitSDK/fit_mesg_with_event_listener.hpp \
    FitSDK/fit_met_zone_mesg.hpp \
    FitSDK/fit_met_zone_mesg_listener.hpp \
    FitSDK/fit_monitoring_hr_data_mesg.hpp \
    FitSDK/fit_monitoring_hr_data_mesg_listener.hpp \
    FitSDK/fit_monitoring_info_mesg.hpp \
    FitSDK/fit_monitoring_info_mesg_listener.hpp \
    FitSDK/fit_monitoring_mesg.hpp \
    FitSDK/fit_monitoring_mesg_listener.hpp \
    FitSDK/fit_nmea_sentence_mesg.hpp \
    FitSDK/fit_nmea_sentence_mesg_listener.hpp \
    FitSDK/fit_obdii_data_mesg.hpp \
    FitSDK/fit_obdii_data_mesg_listener.hpp \
    FitSDK/fit_ohr_settings_mesg.hpp \
    FitSDK/fit_ohr_settings_mesg_listener.hpp \
    FitSDK/fit_one_d_sensor_calibration_mesg.hpp \
    FitSDK/fit_one_d_sensor_calibration_mesg_listener.hpp \
    FitSDK/fit_pad_mesg.hpp \
    FitSDK/fit_pad_mesg_listener.hpp \
    FitSDK/fit_power_zone_mesg.hpp \
    FitSDK/fit_power_zone_mesg_listener.hpp \
    FitSDK/fit_profile.hpp \
    FitSDK/fit_protocol_validator.hpp \
    FitSDK/fit_raw_bbi_mesg.hpp \
    FitSDK/fit_raw_bbi_mesg_listener.hpp \
    FitSDK/fit_record_mesg.hpp \
    FitSDK/fit_record_mesg_listener.hpp \
    FitSDK/fit_respiration_rate_mesg.hpp \
    FitSDK/fit_respiration_rate_mesg_listener.hpp \
    FitSDK/fit_runtime_exception.hpp \
    FitSDK/fit_schedule_mesg.hpp \
    FitSDK/fit_schedule_mesg_listener.hpp \
    FitSDK/fit_sdm_profile_mesg.hpp \
    FitSDK/fit_sdm_profile_mesg_listener.hpp \
    FitSDK/fit_segment_file_mesg.hpp \
    FitSDK/fit_segment_file_mesg_listener.hpp \
    FitSDK/fit_segment_id_mesg.hpp \
    FitSDK/fit_segment_id_mesg_listener.hpp \
    FitSDK/fit_segment_lap_mesg.hpp \
    FitSDK/fit_segment_lap_mesg_listener.hpp \
    FitSDK/fit_segment_leaderboard_entry_mesg.hpp \
    FitSDK/fit_segment_leaderboard_entry_mesg_listener.hpp \
    FitSDK/fit_segment_point_mesg.hpp \
    FitSDK/fit_segment_point_mesg_listener.hpp \
    FitSDK/fit_session_mesg.hpp \
    FitSDK/fit_session_mesg_listener.hpp \
    FitSDK/fit_set_mesg.hpp \
    FitSDK/fit_set_mesg_listener.hpp \
    FitSDK/fit_skin_temp_overnight_mesg.hpp \
    FitSDK/fit_skin_temp_overnight_mesg_listener.hpp \
    FitSDK/fit_slave_device_mesg.hpp \
    FitSDK/fit_slave_device_mesg_listener.hpp \
    FitSDK/fit_sleep_assessment_mesg.hpp \
    FitSDK/fit_sleep_assessment_mesg_listener.hpp \
    FitSDK/fit_sleep_level_mesg.hpp \
    FitSDK/fit_sleep_level_mesg_listener.hpp \
    FitSDK/fit_software_mesg.hpp \
    FitSDK/fit_software_mesg_listener.hpp \
    FitSDK/fit_speed_zone_mesg.hpp \
    FitSDK/fit_speed_zone_mesg_listener.hpp \
    FitSDK/fit_split_mesg.hpp \
    FitSDK/fit_split_mesg_listener.hpp \
    FitSDK/fit_split_summary_mesg.hpp \
    FitSDK/fit_split_summary_mesg_listener.hpp \
    FitSDK/fit_spo2_data_mesg.hpp \
    FitSDK/fit_spo2_data_mesg_listener.hpp \
    FitSDK/fit_sport_mesg.hpp \
    FitSDK/fit_sport_mesg_listener.hpp \
    FitSDK/fit_stress_level_mesg.hpp \
    FitSDK/fit_stress_level_mesg_listener.hpp \
    FitSDK/fit_tank_summary_mesg.hpp \
    FitSDK/fit_tank_summary_mesg_listener.hpp \
    FitSDK/fit_tank_update_mesg.hpp \
    FitSDK/fit_tank_update_mesg_listener.hpp \
    FitSDK/fit_three_d_sensor_calibration_mesg.hpp \
    FitSDK/fit_three_d_sensor_calibration_mesg_listener.hpp \
    FitSDK/fit_time_in_zone_mesg.hpp \
    FitSDK/fit_time_in_zone_mesg_listener.hpp \
    FitSDK/fit_timestamp_correlation_mesg.hpp \
    FitSDK/fit_timestamp_correlation_mesg_listener.hpp \
    FitSDK/fit_totals_mesg.hpp \
    FitSDK/fit_totals_mesg_listener.hpp \
    FitSDK/fit_training_file_mesg.hpp \
    FitSDK/fit_training_file_mesg_listener.hpp \
    FitSDK/fit_unicode.hpp \
    FitSDK/fit_user_profile_mesg.hpp \
    FitSDK/fit_user_profile_mesg_listener.hpp \
    FitSDK/fit_video_clip_mesg.hpp \
    FitSDK/fit_video_clip_mesg_listener.hpp \
    FitSDK/fit_video_description_mesg.hpp \
    FitSDK/fit_video_description_mesg_listener.hpp \
    FitSDK/fit_video_frame_mesg.hpp \
    FitSDK/fit_video_frame_mesg_listener.hpp \
    FitSDK/fit_video_mesg.hpp \
    FitSDK/fit_video_mesg_listener.hpp \
    FitSDK/fit_video_title_mesg.hpp \
    FitSDK/fit_video_title_mesg_listener.hpp \
    FitSDK/fit_watchface_settings_mesg.hpp \
    FitSDK/fit_watchface_settings_mesg_listener.hpp \
    FitSDK/fit_weather_alert_mesg.hpp \
    FitSDK/fit_weather_alert_mesg_listener.hpp \
    FitSDK/fit_weather_conditions_mesg.hpp \
    FitSDK/fit_weather_conditions_mesg_listener.hpp \
    FitSDK/fit_weight_scale_mesg.hpp \
    FitSDK/fit_weight_scale_mesg_listener.hpp \
    FitSDK/fit_workout_mesg.hpp \
    FitSDK/fit_workout_mesg_listener.hpp \
    FitSDK/fit_workout_session_mesg.hpp \
    FitSDK/fit_workout_session_mesg_listener.hpp \
    FitSDK/fit_workout_step_mesg.hpp \
    FitSDK/fit_workout_step_mesg_listener.hpp \
    FitSDK/fit_zones_target_mesg.hpp \
    FitSDK/fit_zones_target_mesg_listener.hpp \
    core/defines.h \
    core/types.h \
    dashboard/dashboard.h \
    mainwindow.h \
    myslide/myslide.h

FORMS += \
    dashboard/dashboard.ui \
    mainwindow.ui \
    myslide/myslide.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    nootbook.txt

DEFINES += QT_DEPRECATED_WARNINGS

#arm64 的编译宏
contains(QMAKE_HOST.arch, aarch64){
    #aarch64 是编译器类型，如果不是这个编译器， 可以把.arch 替换成对应的类型
    #arm 64平台
    message("编译目标平台: arm-64")
    message("编译器平台类型: $$QMAKE_HOST.arch")
}else{
    message("编译目标平台: x86")
    message("编译器平台类型: $$QMAKE_HOST.arch")

    win32{
        # 仅x86-windows下的
        message("win32")
    }

    #仅在linux -x86 平台下的内容
    unix{
        message("unix")
        INCLUDEPATH += \
            /usr/local/ffmpeg/include/
        LIBS += \
            /usr/local/ffmpeg/lib/libavformat.61.dylib \
            /usr/local/ffmpeg/lib/libavcodec.61.dylib \
            /usr/local/ffmpeg/lib/libavdevice.61.dylib \
            /usr/local/ffmpeg/lib/libavfilter.10.dylib \
            /usr/local/ffmpeg/lib/libavutil.59.dylib \
            /usr/local/ffmpeg/lib/libpostproc.58.dylib \
            /usr/local/ffmpeg/lib/libswresample.5.dylib \
            /usr/local/ffmpeg/lib/libswscale.8.dylib
    }
}