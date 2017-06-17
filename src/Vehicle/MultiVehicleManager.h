/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/// @file
///     @author Don Gagne <don@thegagnes.com>

#ifndef MultiVehicleManager_H
#define MultiVehicleManager_H

#include "Vehicle.h"
#include "QGCMAVLink.h"
#include "QmlObjectListModel.h"
#include "QGCToolbox.h"
#include "QGCLoggingCategory.h"

class FirmwarePluginManager;
class FollowMe;
class JoystickManager;
class QGCApplication;
class MAVLinkProtocol;

Q_DECLARE_LOGGING_CATEGORY(MultiVehicleManagerLog)

class GPSRTKFactGroup : public FactGroup
{
    Q_OBJECT

public:
    GPSRTKFactGroup(QObject* parent = NULL);

    Q_PROPERTY(Fact* connected            READ connected            CONSTANT)
    Q_PROPERTY(Fact* currentDuration      READ currentDuration      CONSTANT)
    Q_PROPERTY(Fact* currentAccuracy      READ currentAccuracy      CONSTANT)
    Q_PROPERTY(Fact* valid                READ valid                CONSTANT)
    Q_PROPERTY(Fact* active               READ active               CONSTANT)
    Q_PROPERTY(Fact* numSatellites        READ numSatellites        CONSTANT)

    Fact* connected                    (void) { return &_connected; }
    Fact* currentDuration              (void) { return &_currentDuration; }
    Fact* currentAccuracy              (void) { return &_currentAccuracy; }
    Fact* valid                        (void) { return &_valid; }
    Fact* active                       (void) { return &_active; }
    Fact* numSatellites                (void) { return &_numSatellites; }

    static const char* _connectedFactName;
    static const char* _currentDurationFactName;
    static const char* _currentAccuracyFactName;
    static const char* _validFactName;
    static const char* _activeFactName;
    static const char* _numSatellitesFactName;

private:
    Fact        _connected; ///< is an RTK gps connected?
    Fact        _currentDuration; ///< survey-in status in [s]
    Fact        _currentAccuracy; ///< survey-in accuracy in [mm]
    Fact        _valid; ///< survey-in valid?
    Fact        _active; ///< survey-in active?
    Fact        _numSatellites; ///< number of satellites
};

class MultiVehicleManager : public QGCTool
{
    Q_OBJECT

public:
    MultiVehicleManager(QGCApplication* app, QGCToolbox* toolbox);

    Q_INVOKABLE void        saveSetting (const QString &key, const QString& value);
    Q_INVOKABLE QString     loadSetting (const QString &key, const QString& defaultValue);

    Q_PROPERTY(bool                 activeVehicleAvailable          READ activeVehicleAvailable                                         NOTIFY activeVehicleAvailableChanged)
    Q_PROPERTY(bool                 parameterReadyVehicleAvailable  READ parameterReadyVehicleAvailable                                 NOTIFY parameterReadyVehicleAvailableChanged)
    Q_PROPERTY(Vehicle*             activeVehicle                   READ activeVehicle                  WRITE setActiveVehicle          NOTIFY activeVehicleChanged)
    Q_PROPERTY(QmlObjectListModel*  vehicles                        READ vehicles                                                       CONSTANT)
    Q_PROPERTY(bool                 gcsHeartBeatEnabled             READ gcsHeartbeatEnabled            WRITE setGcsHeartbeatEnabled    NOTIFY gcsHeartBeatEnabledChanged)
    Q_PROPERTY(FactGroup*           gpsRtk                          READ gpsRtkFactGroup                CONSTANT)

    /// A disconnected vehicle used for offline editing. It will match the vehicle type specified in Settings.
    Q_PROPERTY(Vehicle*             offlineEditingVehicle           READ offlineEditingVehicle                                          CONSTANT)

    // Methods

    Q_INVOKABLE Vehicle* getVehicleById(int vehicleId);

    UAS* activeUas(void) { return _activeVehicle ? _activeVehicle->uas() : NULL; }

    // Property accessors

    bool activeVehicleAvailable(void) { return _activeVehicleAvailable; }

    bool parameterReadyVehicleAvailable(void) { return _parameterReadyVehicleAvailable; }

    Vehicle* activeVehicle(void) { return _activeVehicle; }
    void setActiveVehicle(Vehicle* vehicle);

    QmlObjectListModel* vehicles(void) { return &_vehicles; }

    bool gcsHeartbeatEnabled(void) const { return _gcsHeartbeatEnabled; }
    void setGcsHeartbeatEnabled(bool gcsHeartBeatEnabled);

    Vehicle* offlineEditingVehicle(void) { return _offlineEditingVehicle; }

    /// Determines if the link is in use by a Vehicle
    ///     @param link Link to test against
    ///     @param skipVehicle Don't consider this Vehicle as part of the test
    /// @return true: link is in use by one or more Vehicles
    bool linkInUse(LinkInterface* link, Vehicle* skipVehicle);

    // Override from QGCTool
    virtual void setToolbox(QGCToolbox *toolbox);

    FactGroup* gpsRtkFactGroup         (void) { return &_gpsRtkFactGroup; }

signals:
    void vehicleAdded(Vehicle* vehicle);
    void vehicleRemoved(Vehicle* vehicle);
    void activeVehicleAvailableChanged(bool activeVehicleAvailable);
    void parameterReadyVehicleAvailableChanged(bool parameterReadyVehicleAvailable);
    void activeVehicleChanged(Vehicle* activeVehicle);
    void gcsHeartBeatEnabledChanged(bool gcsHeartBeatEnabled);

    void _deleteVehiclePhase2Signal(void);

private slots:
    void _deleteVehiclePhase1(Vehicle* vehicle);
    void _deleteVehiclePhase2(void);
    void _setActiveVehiclePhase2(void);
    void _vehicleParametersReadyChanged(bool parametersReady);
    void _sendGCSHeartbeat(void);
    void _vehicleHeartbeatInfo(LinkInterface* link, int vehicleId, int componentId, int vehicleMavlinkVersion, int vehicleFirmwareType, int vehicleType);

    void _onGPSConnect();
    void _onGPSDisconnect();
    void _GPSSurveyInStatus(float duration, float accuracyMM, bool valid, bool active);
    void _GPSNumSatellites(int numSatellites);

private:
    bool _vehicleExists(int vehicleId);

    bool        _activeVehicleAvailable;            ///< true: An active vehicle is available
    bool        _parameterReadyVehicleAvailable;    ///< true: An active vehicle with ready parameters is available
    Vehicle*    _activeVehicle;                     ///< Currently active vehicle from a ui perspective
    Vehicle*    _offlineEditingVehicle;             ///< Disconnected vechicle used for offline editing
    GPSRTKFactGroup _gpsRtkFactGroup;               ///< RTK GPS information

    QList<Vehicle*> _vehiclesBeingDeleted;          ///< List of Vehicles being deleted in queued phases
    Vehicle*        _vehicleBeingSetActive;         ///< Vehicle being set active in queued phases

    QList<int>  _ignoreVehicleIds;          ///< List of vehicle id for which we ignore further communication

    QmlObjectListModel  _vehicles;

    FirmwarePluginManager*      _firmwarePluginManager;
    JoystickManager*            _joystickManager;
    MAVLinkProtocol*            _mavlinkProtocol;

    QTimer              _gcsHeartbeatTimer;             ///< Timer to emit heartbeats
    bool                _gcsHeartbeatEnabled;           ///< Enabled/disable heartbeat emission
    static const int    _gcsHeartbeatRateMSecs = 1000;  ///< Heartbeat rate
    static const char*  _gcsHeartbeatEnabledKey;
};

#endif
