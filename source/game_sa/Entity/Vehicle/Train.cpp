/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#include "StdInc.h"

#include "Train.h"

#include "Buoyancy.h"
#include "CarCtrl.h"

CVector CTrain::aStationCoors[6] = { // 0x8D48F8
    CVector{ 1741.0f, -1954.0f, 15.0f },
    CVector{ 1297.0f, -1898.0f, 3.0f  },
    CVector{ -1945.0f, 128.0f,  29.0f },
    CVector{ 1434.0f,  2632.0f, 13.0f },
    CVector{ 2783.0f,  1758.0f, 12.0f },
    CVector{ 2865.0f,  1281.0f, 12.0  }
};

auto& pTrackNodes = StaticRef<CTrainNode*[4]>(0xC38024);
auto& NumTrackNodes = StaticRef<std::array<int32, 4>>(0xC38014);
auto& TotalLengthOfTrack = StaticRef<std::array<float, 4>>(0xC37FEC);
auto& StationDist = StaticRef<std::array<float, 6>>(0xC38034);

void CTrain::InjectHooks() {
    RH_ScopedVirtualClass(CTrain, 0x872370, 66);
    RH_ScopedCategory("Vehicle");

    RH_ScopedInstall(Constructor, 0x6F6030, { .reversed = false });
    RH_ScopedInstall(InitTrains, 0x6F7440, { .reversed = false });
    RH_ScopedInstall(ReadAndInterpretTrackFile, 0x6F55D0);
    RH_ScopedInstall(Shutdown, 0x6F58D0);
    RH_ScopedInstall(UpdateTrains, 0x6F5900);
    RH_ScopedInstall(FindCoorsFromPositionOnTrack, 0x6F59E0, { .reversed = false });
    RH_ScopedInstall(FindMaximumSpeedToStopAtStations, 0x6F5BA0, { .reversed = false });
    RH_ScopedInstall(FindNumCarriagesPulled, 0x6F5CD0);
    RH_ScopedInstall(OpenTrainDoor, 0x6F5D80);
    RH_ScopedInstall(AddPassenger, 0x6F5D90);
    RH_ScopedInstall(RemovePassenger, 0x6F5DA0);
    RH_ScopedInstall(DisableRandomTrains, 0x6F5DB0);
    RH_ScopedInstall(RemoveOneMissionTrain, 0x6F5DC0);
    RH_ScopedInstall(ReleaseOneMissionTrain, 0x6F5DF0);
    RH_ScopedInstall(SetTrainSpeed, 0x6F5E20);
    RH_ScopedInstall(SetTrainCruiseSpeed, 0x6F5E50);
    RH_ScopedInstall(FindCaboose, 0x6F5E70);
    RH_ScopedInstall(FindEngine, 0x6F5E90);
    RH_ScopedInstall(FindCarriage, 0x6F5EB0);
    RH_ScopedInstall(FindSideStationIsOn, 0x6F5EF0);
    RH_ScopedInstall(FindNextStationPositionInDirection, 0x6F5F00);
    RH_ScopedInstall(IsInTunnel, 0x6F6320);
    RH_ScopedInstall(RemoveRandomPassenger, 0x6F6850, { .reversed = false });
    RH_ScopedInstall(RemoveMissionTrains, 0x6F6A20);
    RH_ScopedInstall(RemoveAllTrains, 0x6F6AA0);
    RH_ScopedInstall(ReleaseMissionTrains, 0x6F6B60);
    RH_ScopedInstall(FindClosestTrackNode, 0x6F6BD0);
    RH_ScopedInstall(FindPositionOnTrackFromCoors, 0x6F6CC0, { .reversed = false });
    RH_ScopedInstall(FindNearestTrain, 0x6F7090);
    RH_ScopedInstall(SetNewTrainPosition, 0x6F7140);
    RH_ScopedInstall(IsNextStationAllowed, 0x6F7260);
    RH_ScopedInstall(SkipToNextAllowedStation, 0x6F72F0);
    RH_ScopedInstall(CreateMissionTrain, 0x6F7550, { .reversed = false });
    RH_ScopedInstall(DoTrainGenerationAndRemoval, 0x6F7900, { .reversed = false });
    RH_ScopedInstall(AddNearbyPedAsRandomPassenger, 0x6F8170, { .reversed = false });
    RH_ScopedVMTInstall(ProcessControl, 0x6F86A0);

    RH_ScopedGlobalInstall(ProcessTrainAnnouncements, 0x6F5910);
    RH_ScopedGlobalInstall(PlayAnnouncement, 0x6F5920);
    RH_ScopedGlobalInstall(MarkSurroundingEntitiesForCollisionWithTrain, 0x6F6640);
    RH_ScopedGlobalInstall(TrainHitStuff<CPtrListSingleLink<CPhysical*>>, 0x6F5CF0, { .reversed = false });
}

// 0x6F6030
CTrain::CTrain(int32 modelIndex, eVehicleCreatedBy createdBy) : CVehicle(createdBy) {
    plugin::CallMethod<0x6F6030, CTrain*, int32, eVehicleCreatedBy>(this, modelIndex, createdBy);
    return;

    m_baseVehicleType = m_vehicleType = VEHICLE_TYPE_TRAIN;
    const auto mi = CModelInfo::GetModelInfo(modelIndex)->AsVehicleModelInfoPtr();
    m_pHandlingData = gHandlingDataMgr.GetVehiclePointer(mi->m_nHandlingId);
    m_nHandlingFlagsIntValue = m_pHandlingData->m_nHandlingFlags;

    CVehicle::SetModelIndex(modelIndex);
    SetupModelNodes();

    std::memset(&Door, 0, sizeof(Door));
    if (m_nModelIndex == MODEL_STREAKC) {
        Door[DOOR_LEFT_FRONT].Init(1.25f, 0.25f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_EXTRA_BASED);
        Door[DOOR_RIGHT_FRONT].Init(1.25f, 0.25f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_EXTRA_BASED);
    } else {
        Door[DOOR_LEFT_FRONT].Init(TWO_PI / -5.0f, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_EXTRA_BASED);
        Door[DOOR_RIGHT_FRONT].Init(TWO_PI / +5.0f, 0.0f, DOOR_AXIS_NEG_Y, DOOR_AXIS_Z, DOOR_EXTRA_BASED);
    }

    { // todo:
    m_nTrainFlags.bDirection = true;
    m_nTrainFlags.bCaboose = true;
    m_nTrainFlags.bEngine = true;
    m_nTrainFlags.bStopForStations = true;
    }

    m_PassengersMode = 0;
    m_NumPassengersToBoard = CGeneral::GetRandomNumber() & 3; // ?
    m_NumPassengersOnTrain = (CGeneral::GetRandomNumber() & 3) + 1;
    m_pRandomPedForTrain = nullptr;
    m_nMaxPassengers = 5;
    m_nPhysicalFlags.bDontProcessCollisionOurSelves = true;
    SetUsesCollision(true);
    m_nDoorTimer = CTimer::GetTimeInMS();
    DoorState = 0;
    TrainType = 0;
    PositionOnTrack = 0.0f;
    LinearSpeed = 0.0f;
    m_StopAtStationTimer = 0;
    mi->ChooseVehicleColour(m_nPrimaryColor, m_nSecondaryColor, m_nTertiaryColor, m_nQuaternaryColor, 1);
    m_fMass = m_pHandlingData->m_fMass;
    m_fTurnMass = m_pHandlingData->m_fTurnMass;
    m_vecCentreOfMass = m_pHandlingData->m_vecCentreOfMass;
    m_fElasticity = 0.05f;
    m_fBuoyancyConstant = m_pHandlingData->m_fBuoyancyConstant;
    m_fAirResistance = GetDefaultAirResistance();

    m_nPhysicalFlags.bUsingSpecialColModel = true;
    m_bTunnelTransition = true;
    pLinkedToForward = nullptr;
    pLinkedToBackward = nullptr;
    SetStatus(STATUS_TRAIN_MOVING);
    m_autoPilot.ActualSpeed = 0.0f;
    m_autoPilot.SetCruiseSpeed(0);
    m_vehicleAudio.Initialise(this);
}

void CTrain::SetupModelNodes() {
    std::ranges::fill(m_aTrainNodes, nullptr);
    CClumpModelInfo::FillFrameArray(GetRpClump(), m_aTrainNodes.data());
}

// 0x6F7440
void CTrain::InitTrains() {
    ZoneScoped;

    return plugin::Call<0x6F7440>();

    bDisableRandomTrains = false;
    GenTrain_Status = 0;

    constexpr const char* filenames[] {
        "data\\paths\\tracks.dat",
        "data\\paths\\tracks3.dat",
        "data\\paths\\tracks2.dat",
        "data\\paths\\tracks4.dat",
    };
    for (auto i = 0u; i < std::size(pTrackNodes); ++i) {
        if (!pTrackNodes[i]) {
            CTrain::ReadAndInterpretTrackFile(filenames[i], pTrackNodes, NumTrackNodes.data(), TotalLengthOfTrack.data(), i);
        }
    }

    for (auto i = 0u; i < std::size(aStationCoors); ++i) {
        int32 trackId;
        CTrain::FindClosestTrackNode(aStationCoors[i], &trackId);
        auto distance = (float)pTrackNodes[trackId]->m_nDistanceFromStart;
        StationDist[i] = distance / 3.0f;
    }
}

// 0x6F55D0
// ASM Match: 99.4%
void CTrain::ReadAndInterpretTrackFile(const char* filename, CTrainNode** nodes, int32* lineCount, float* totalDist, int32 skipStations)
{
    if (*nodes == nullptr)
    {
        const int32 nFileSize = 46384;
        uint8* pFileBuffer = new uint8[nFileSize];
        CFileMgr::LoadFile(filename, pFileBuffer, nFileSize, "rb");

        int32 fpos = 0;
        int32 pos = 0;
        while (pFileBuffer[fpos] != '\n')
        {
            gString[pos++] = pFileBuffer[fpos++];
        }
        gString[pos] = '\0';
        fpos++;

        if (strcmp("processed", gString) == 0)
        {
            pos = 0;
            while (pFileBuffer[fpos] != '\n')
            {
                gString[pos++] = pFileBuffer[fpos++];
            }
            gString[pos] = '\0';
            fpos++;
        }

        sscanf(gString, "%d", lineCount);
        *nodes = new CTrainNode[*lineCount];

        for (int32 i = 0, C = 0; i < *lineCount; ++i)
        {
            pos = 0;
            while (pFileBuffer[fpos] != '\n')
            {
                gString[pos++] = pFileBuffer[fpos++];
            }
            fpos++;

            int32 Station;
            float fX;
            float fY;
            float fZ;
            sscanf(gString, "%f %f %f %d", &fX, &fY, &fZ, &Station);

            CTrainNode*& trackNodes = *nodes;
            trackNodes[i].SetCoorsX(fX);
            trackNodes[i].SetCoorsY(fY);
            trackNodes[i].SetCoorsZ(fZ);

            if (skipStations == TTYPE_MAIN && Station != 0)
            {
                aStationCoors[C++] = CVector(fX, fY, fZ);
            }
        }
        delete[] pFileBuffer;
    }

    float Length = 0.0f;
    CTrainNode*& trackNodes = *nodes;

    for (int32 C = 0; C < *lineCount; ++C)
    {
        trackNodes[C].SetLengthFromStart(Length);

        Length += CMaths::Sqrt((trackNodes[C].GetCoorsX() - trackNodes[(C + 1) % *lineCount].GetCoorsX()) *
                                   (trackNodes[C].GetCoorsX() - trackNodes[(C + 1) % *lineCount].GetCoorsX()) +
                               (trackNodes[C].GetCoorsY() - trackNodes[(C + 1) % *lineCount].GetCoorsY()) *
                                   (trackNodes[C].GetCoorsY() - trackNodes[(C + 1) % *lineCount].GetCoorsY()));
    }

    *totalDist = Length;
}

// 0x6F58D0
void CTrain::Shutdown() {
    for (auto node : pTrackNodes) {
        delete node;
        node = nullptr;
    }
}

// 0x6F5900
void CTrain::UpdateTrains() {
    ZoneScoped;

    // NOP
}

// 0x6F5910
void ProcessTrainAnnouncements() {
    // NOP
}

// 0x6F5920
void PlayAnnouncement(uint8 arg0, uint8 arg1) {
    // NOP
}

// 0x6F59E0
void CTrain::FindCoorsFromPositionOnTrack(float railDistance, int32 trackId, CVector* outCoors) {
    ((void(__cdecl*)(float, int32, CVector*))0x6F59E0)(railDistance, trackId, outCoors);
}

// 0x6F5BA0
bool CTrain::FindMaximumSpeedToStopAtStations(float* speed) {
    return ((bool(__thiscall*)(CTrain*, float*))0x6F5BA0)(this, speed);
}

// 0x6F5CD0
uint32 CTrain::FindNumCarriagesPulled() {
    uint32 num;
    CTrain* carriage = pLinkedToBackward;
    for (num = 0; carriage; ++num) {
        carriage = carriage->pLinkedToBackward;
    }
    return num;
}

// 0x6F5D80
void CTrain::OpenTrainDoor(float state) {
    // NOP
}

// 0x6F5D90
void CTrain::AddPassenger(CPed* ped) {
    // NOP
}

// 0x6F5DA0
void CTrain::RemovePassenger(CPed* ped) {
    // NOP
}

// 0x6F5DB0
void CTrain::DisableRandomTrains(bool disable) {
    bDisableRandomTrains = disable;
}

// 0x6F5DC0
void CTrain::RemoveOneMissionTrain(CTrain* train) {
    if (!train)
        return;

    CTrain* next;
    CTrain* _train = train;
    do {
        next = _train->pLinkedToBackward;
        CWorld::Remove(_train);
        delete _train;
        _train = next;
    } while (next);
}

// 0x6F5DF0
void CTrain::ReleaseOneMissionTrain(CTrain* train) {
    for (auto* head = train; head; head = head->pLinkedToBackward) {
        head->m_nTrainFlags.bMissionTrain = false;
    }
}

// 0x6F5E20
void CTrain::SetTrainSpeed(CTrain* train, float speed) {
    train->LinearSpeed = speed / 50.0f;
    if (!train->m_nTrainFlags.bDirection) {
        train->LinearSpeed = -train->LinearSpeed;
    }
}

// 0x6F5E50
void CTrain::SetTrainCruiseSpeed(CTrain* train, float speed) {
    train->m_autoPilot.SetCruiseSpeed((uint8)speed);
}

// 0x6F5E70
// ASM Match
CTrain* CTrain::FindCaboose(CTrain* train)
{
    CTrain* pTrain = train;
    while (pTrain->pLinkedToBackward != nullptr)
    {
        pTrain = pTrain->pLinkedToBackward;
    }
    return pTrain;
}

// 0x6F5E90
// ASM Match
CTrain* CTrain::FindEngine(CTrain* train)
{
    CTrain* pTrain = train;
    while (pTrain->pLinkedToForward != nullptr)
    {
        pTrain = pTrain->pLinkedToForward;
    }
    return pTrain;
}

// 0x6F5EB0
// ASM Match
CTrain* CTrain::FindCarriage(CTrain* train, uint8 carriage)
{
    CTrain* pTrain = train;
    for (uint8 CurrentCarriageNumber = 0; CurrentCarriageNumber < carriage; ++CurrentCarriageNumber)
    {
        pTrain = pTrain->pLinkedToBackward;
        if (pTrain == nullptr)
        {
            return nullptr;
        }
    }
    return pTrain;
}

// 0x6F5EF0
bool CTrain::FindSideStationIsOn() const {
    return m_nTrainFlags.bDirection; // ?
}

// 0x6F5F00
// ASM Match: 99.53%
void CTrain::FindNextStationPositionInDirection(bool clockwiseDirection, float distance, float* distanceToStation, int32* numStations)
{
    int32 Station = 0;

    while (Station < MAX_STATIONS && !(StationDist[Station] > distance))
    {
        ++Station;
    }

    if (Station >= MAX_STATIONS)
    {
        Station = 0;
    }
    if (!clockwiseDirection && --Station < 0)
    {
        Station += 6;
    }

    if (CMaths::Abs(distance - StationDist[Station]) < 100.0f)
    {
        Station = clockwiseDirection ? (Station + 1) : (Station - 1);

        if (Station < 0)
        {
            Station += MAX_STATIONS;
        }
        if (Station >= MAX_STATIONS)
        {
            Station = 0;
        }
    }

    *numStations = Station;
    *distanceToStation = StationDist[Station];
}

// 0x6F6320
bool CTrain::IsInTunnel() const {
    const auto& pos = GetPosition();
    CColPoint colPoint{};
    CEntity* colEntity{};
    return CWorld::ProcessVerticalLine(pos, pos.z + 100.0f, colPoint, colEntity, true);
}

// 0x6F6640
void MarkSurroundingEntitiesForCollisionWithTrain(CVector pos, float radius, CEntity* entity, bool bOnlyVehicles) {
    int32 startSectorX = std::max(CWorld::GetSectorX(pos.x - radius), 0);
    int32 startSectorY = std::max(CWorld::GetSectorY(pos.y - radius), 0);
    int32 endSectorX   = std::min(CWorld::GetSectorX(pos.x + radius), MAX_SECTORS_X - 1);
    int32 endSectorY   = std::min(CWorld::GetSectorY(pos.y + radius), MAX_SECTORS_Y - 1);

    CWorld::AdvanceCurrentScanCode();

    for (int32 sectorY = startSectorY; sectorY <= endSectorY; ++sectorY) {
        for (int32 sectorX = startSectorX; sectorX <= endSectorX; ++sectorX) {
            auto& repeatSector = CWorld::GetRepeatSector(sectorX, sectorY);
            TrainHitStuff(repeatSector.Vehicles, entity);
            if (!bOnlyVehicles) {
                TrainHitStuff(repeatSector.Peds, entity);
                TrainHitStuff(repeatSector.Objects, entity);
            }
        }
    }
}

// 0x6F5CF0
template<typename PtrListType>
void TrainHitStuff(PtrListType& ptrList, CEntity* entity) {
    ((void(__cdecl*)(PtrListType&, CEntity*))0x6F5CF0)(ptrList, entity);
}

// 0x6F6850
void CTrain::RemoveRandomPassenger() {
    ((void(__thiscall*)(CTrain*))0x6F6850)(this);
}

// 0x6F6A20
void CTrain::RemoveMissionTrains() {
    for (auto& vehicle : GetVehiclePool()->GetAllValid()) {
        if (vehicle.IsTrain() &&
            &vehicle != FindPlayerVehicle() &&
            vehicle.AsTrain()->m_nTrainFlags.bMissionTrain
        ) {
            CWorld::Remove(&vehicle);
            delete &vehicle;
        }
    }
}

// 0x6F6AA0
// ASM Match: 99.93%
void CTrain::RemoveAllTrains()
{
    CVehiclePool& VehiclePool = *GetVehiclePool();
    CVehicle* pVeh;

    for (int32 i = VehiclePool.GetSize(); i != 0;)
    {
        pVeh = VehiclePool.GetSlot(--i);
        if (pVeh != nullptr && pVeh->GetBaseVehicleType() == VEHICLE_TYPE_TRAIN)
        {
            bool bPartOfPlayerTrain = false;
            CTrain* pTrain = static_cast<CTrain*>(pVeh);
            do
            {
                if (pTrain == FindPlayerVehicle())
                {
                    bPartOfPlayerTrain = true;
                }
                pTrain = pTrain->pLinkedToForward;
            } while (pTrain);

            pTrain = static_cast<CTrain*>(pVeh);
            do
            {
                if (pTrain == FindPlayerVehicle())
                {
                    bPartOfPlayerTrain = true;
                }
                pTrain = pTrain->pLinkedToBackward;
            } while (pTrain);

            if (!bPartOfPlayerTrain)
            {
                CWorld::Remove(pVeh);
                delete pVeh;
            }
        }
    }
}

// 0x6F6B60
void CTrain::ReleaseMissionTrains() {
    for (auto& vehicle : GetVehiclePool()->GetAllValid()) {
        if (vehicle.IsTrain() && &vehicle != FindPlayerVehicle()) {
            vehicle.AsTrain()->m_nTrainFlags.bMissionTrain = false;
        }
    }
}

// 0x6F6BD0
// ASM Match: 99.59%
int32 CTrain::FindClosestTrackNode(CVector posn, int32* outTrackId)
{
    int32 TestNode, ClosestNode;
    float ClosestDist, Dist;
    int32 TrackType;

    ClosestDist = 99999.9f;
    for (TrackType = 0; TrackType < TTYPE_NUMBEROFTHEM; TrackType++)
    {
        for (TestNode = 0; TestNode < NumTrackNodes[TrackType]; TestNode++)
        {
            Dist = (posn - CVector(pTrackNodes[TrackType][TestNode].GetCoorsX(), pTrackNodes[TrackType][TestNode].GetCoorsY(), pTrackNodes[TrackType][TestNode].GetCoorsZ())).Magnitude();
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                ClosestNode = TestNode;
                *outTrackId = TrackType;
            }
        }
    }

    return ClosestNode;
}

// 0x6F6CC0
void CTrain::FindPositionOnTrackFromCoors() {
    ((void(__thiscall*)(CTrain*))0x6F6CC0)(this);
}

// 0x6F7090
// ASM Match: 99.92%
CTrain* CTrain::FindNearestTrain(CVector posn, bool mustBeMainTrain)
{
    CVehiclePool& VehiclePool = *GetVehiclePool();
    CTrain* pResult = nullptr;
    float NearestDist = 10000000.0f;

    for (int32 i = VehiclePool.GetSize(); i != 0;)
    {
        CVehicle* pVeh = VehiclePool.GetSlot(--i);
        if (pVeh != nullptr && pVeh->GetBaseVehicleType() == VEHICLE_TYPE_TRAIN)
        {
            CTrain* pTrain = static_cast<CTrain*>(pVeh);
            float Dist = (pTrain->GetPosition() - posn).Magnitude2D();
            if (Dist < NearestDist && (mustBeMainTrain == false || pTrain->m_nTrainFlags.bEngine))
            {
                NearestDist = Dist;
                pResult = pTrain;
            }
        }
    }

    return pResult;
}

// 0x6F7140
void CTrain::SetNewTrainPosition(CTrain* train, CVector posn) {
    train->SetPosn(posn);
    train->FindPositionOnTrackFromCoors();
}

// 0x6F7260
// ASM Match: 99.78%
bool CTrain::IsNextStationAllowed(CTrain* train)
{
    CTrain* pEngine = FindEngine(train);

    int32 Station;
    float PosOnTrack = pEngine->PositionOnTrack;

    FindNextStationPositionInDirection(pEngine->FindSideStationIsOn(), PosOnTrack, &PosOnTrack, &Station);
    eLevelName Level = CTheZones::GetLevelFromPosition(aStationCoors[Station]);

    return static_cast<float>(Level) <= CStats::GetStatValue(STAT_CITY_UNLOCKED) + 1.0f ? true : false;
}

// 0x6F72F0
// ASM Match: 99.62%
void CTrain::SkipToNextAllowedStation(CTrain* train)
{
    CTrain* pEngine = FindEngine(train);

    int32 Station;
    float PosOnTrack = pEngine->PositionOnTrack;

    while (true)
    {
        FindNextStationPositionInDirection(pEngine->FindSideStationIsOn(), PosOnTrack, &PosOnTrack, &Station);
        eLevelName Level = CTheZones::GetLevelFromPosition(aStationCoors[Station]);

        if (static_cast<float>(Level) <= CStats::GetStatValue(STAT_CITY_UNLOCKED) + 1.0f)
        {
            break;
        }
    }

    pEngine->PositionOnTrack = PosOnTrack;

    if (pEngine->FindSideStationIsOn())
    {
        pEngine->PositionOnTrack -= 20.0f;
        pEngine->LinearSpeed = 0.1f;
    }
    else
    {
        pEngine->PositionOnTrack += 20.0f;
        pEngine->LinearSpeed = -0.1f;
    }

    CStreaming::LoadScene(aStationCoors[Station]);
    CStreaming::LoadAllRequestedModels(false);
    CGameLogic::PassTime((uint32)(((aStationCoors[Station] - pEngine->GetPosition()).Magnitude2D() / 20) + 23));
}

// 0x6F7550
void CTrain::CreateMissionTrain(CVector posn, bool clockwiseDirection, uint32 trainType, CTrain** outFirstCarriage, CTrain** outLastCarriage, int32 nodeIndex, int32 trackId, bool isMissionTrain) {
    ((void(__cdecl*)(CVector, bool, uint32, CTrain**, CTrain**, int32, int32, bool))0x6F7550)(posn, clockwiseDirection, trainType, outFirstCarriage, outLastCarriage, nodeIndex, trackId, isMissionTrain);
}

// 0x6F7900
void CTrain::DoTrainGenerationAndRemoval() {
    ((void(__cdecl*)())0x6F7900)();
}

// 0x6F8170
void CTrain::AddNearbyPedAsRandomPassenger() {
    ((void(__thiscall*)(CTrain*))0x6F8170)(this);
}

// 0x6F86A0
void CTrain::ProcessControl() {
    vehicleFlags.bWarnedPeds = 0;
    m_vehicleAudio.Service();
    if (gbModelViewer) {
        return;
    }

    CVector vecOldTrainPosition = GetPosition();
    float fOldTrainHeading = GetHeading();

    const float& fTotalTrackLength = TotalLengthOfTrack[TrainType];
    CTrainNode* trainNodes = pTrackNodes[TrainType];
    auto numTrackNodes = NumTrackNodes[TrainType];

    if (m_nTrainFlags.bDerailed == 0) {
        if (!m_nTrainFlags.bEngine) {
            if (pLinkedToForward) {
                LinearSpeed = pLinkedToForward->LinearSpeed;
                PositionOnTrack = pLinkedToForward->PositionOnTrack + OffsetFromLeader;
            } else {
                LinearSpeed *= std::pow(0.9900000095367432f, CTimer::GetTimeStep());
                PositionOnTrack += LinearSpeed * CTimer::GetTimeStep();
            }

            if (m_nTrainFlags.bDoorsReady && m_nTrainFlags.bAtStation && m_nModelIndex == MODEL_STREAKC && !m_nTrainFlags.bMissionTrain) {
                CPlayerPed* localPlayer = FindPlayerPed();
                if (m_PassengersMode == TRAIN_PASSENGERS_QUERY_NUM_PASSENGERS_TO_LEAVE) {
                    if (localPlayer->m_pMyVehicle == this) {
                        m_NumPassengersOnTrain = 0;
                    } else {
                        m_NumPassengersOnTrain = (CGeneral::GetRandomNumber() & 3) + 1; // [1, 4]
                    }
                    m_PassengersMode = TRAIN_PASSENGERS_TELL_PASSENGERS_TO_LEAVE;
                }

                if (m_PassengersMode == TRAIN_PASSENGERS_TELL_PASSENGERS_TO_LEAVE) {
                    RemoveRandomPassenger();
                    if (m_NumPassengersOnTrain == 0) {
                        m_PassengersMode = TRAIN_PASSENGERS_QUERY_NUM_PASSENGERS_TO_ENTER;
                    }
                }

                if (m_PassengersMode == TRAIN_PASSENGERS_QUERY_NUM_PASSENGERS_TO_ENTER) {
                    if (localPlayer->m_pMyVehicle == this) {
                        m_NumPassengersToBoard = 0;
                    } else {
                        m_NumPassengersToBoard = CGeneral::GetRandomNumber() % 4 + 1; // rand(1, 4)
                    }
                    m_PassengersMode = TRAIN_PASSENGERS_TELL_PASSENGERS_TO_ENTER;
                }

                if (m_PassengersMode == TRAIN_PASSENGERS_TELL_PASSENGERS_TO_ENTER) {
                    if (m_nTrainFlags.bPassengersCanBoard) {
                        AddNearbyPedAsRandomPassenger();
                        if (m_NumPassengersOnTrain == m_NumPassengersToBoard) {
                            m_PassengersMode = TRAIN_PASSENGERS_GENERATION_FINISHED;
                        }
                    }
                }
            }
        } else {
            CPad* pad = CPad::GetPad();
            if (m_pDriver && m_pDriver->IsPlayer()) {
                pad = m_pDriver->AsPlayer()->GetPadFromPlayer();
            }

            uint32 numCarriagesPulled = FindNumCarriagesPulled();
            if (!m_nTrainFlags.bDirection) {
                LinearSpeed = -LinearSpeed;
            }

            if (GetStatus()) {
                bool bHasPassengerCarriages = m_nTrainFlags.bHasPassengerCarriages;
                auto fStopAtStationSpeed = static_cast<float>(m_autoPilot.CruiseSpeed);

                uint32 timeAtStation = CTimer::GetTimeInMS() - m_StopAtStationTimer;
                if (timeAtStation >= (bHasPassengerCarriages ? 20'000u : 10'000u)) {
                    if (timeAtStation >= (bHasPassengerCarriages ? 28'000u : 18'000u)) {
                        if (timeAtStation >= (bHasPassengerCarriages ? 32'000u : 22'000u)) {
                            if (m_nTrainFlags.bStopForStations) {
                                float maxTrainSpeed = 0.0f;
                                if (FindMaximumSpeedToStopAtStations(&maxTrainSpeed)) {
                                    fStopAtStationSpeed = 0.0f;
                                    m_StopAtStationTimer = CTimer::GetTimeInMS();
                                } else {
                                    if (fStopAtStationSpeed >= maxTrainSpeed) {
                                        fStopAtStationSpeed = maxTrainSpeed;
                                    }
                                }
                            }
                        } else if (m_nTrainFlags.bAtStation) {
                            CTrain* trainCarriage = this;
                            do {
                                m_nTrainFlags.bAtStation = false;
                                trainCarriage->m_PassengersMode = TRAIN_PASSENGERS_GENERATION_FINISHED;
                                trainCarriage = trainCarriage->pLinkedToBackward;
                            } while (trainCarriage);
                        }
                    } else {
                        fStopAtStationSpeed = 0.0f;
                        if (m_nTrainFlags.bAtStation) {
                            CTrain* trainCarriage = this;
                            do {
                                m_nTrainFlags.bPassengersCanBoard = false;
                                trainCarriage->m_PassengersMode = TRAIN_PASSENGERS_GENERATION_FINISHED;
                                trainCarriage = trainCarriage->pLinkedToBackward;
                            } while (trainCarriage);
                        }
                    }
                } else {
                    fStopAtStationSpeed = 0.0f;
                    if (!m_nTrainFlags.bAtStation) {
                        CTrain* trainCarriage = this;
                        do {
                            m_nTrainFlags.bAtStation = true;
                            m_nTrainFlags.bPassengersCanBoard = true;
                            trainCarriage->m_PassengersMode = TRAIN_PASSENGERS_QUERY_NUM_PASSENGERS_TO_LEAVE;
                            trainCarriage = trainCarriage->pLinkedToBackward;
                        } while (trainCarriage);
                    }
                }

                fStopAtStationSpeed = fStopAtStationSpeed / 50.0f - LinearSpeed;
                if (fStopAtStationSpeed > 0.0f) {
                    Gas = fStopAtStationSpeed * 30.0f;
                    if (Gas >= 1.0f) {
                        Gas = 1.0f;
                    }

                    Gas *= 255.0f;
                    Brake = 0.0f;
                } else {
                    float fTrainSpeed = fStopAtStationSpeed * -30.0f;
                    Gas = 0.0f;
                    if (fTrainSpeed >= 1.0f) {
                        fTrainSpeed = 1.0f;
                    }
                    Brake = fTrainSpeed * 255.0f;
                }
            } else {
                float fTrainSpeed = LinearSpeed;
                if (fTrainSpeed < 0.0f) {
                    fTrainSpeed = -fTrainSpeed;
                }

                if (fTrainSpeed < 0.001f) {
                    Brake = 0.0f;
                    Gas = static_cast<float>(pad->GetAccelerate() - pad->GetBrake());
                } else {
                    if (LinearSpeed > 0.0f) {
                        Brake = static_cast<float>(pad->GetBrake());
                        Gas = static_cast<float>(pad->GetAccelerate());
                    } else {
                        Gas = static_cast<float>(-pad->GetBrake());
                        Brake = static_cast<float>(pad->GetAccelerate());
                    }
                }
            }

            if (m_nTrainFlags.bIsForcedToSlowDown) {
                const CVector& vecPoint = GetPosition();
                CVector vecDistance{};
                if (CGameLogic::CalcDistanceToForbiddenTrainCrossing(vecPoint, m_vecMoveSpeed, true, vecDistance) < 230.0f) {
                    if (DotProduct(GetForwardVector(), vecDistance) <= 0.0f) {
                        Gas = std::max(0.0f, Gas);
                    } else {
                        Gas = std::min(0.0f, Gas);
                    }

                    if (CGameLogic::CalcDistanceToForbiddenTrainCrossing(vecPoint, m_vecMoveSpeed, false, vecDistance) < 230.0f) {
                        Brake = 512.0f;
                    }
                }
            }

            numCarriagesPulled += 3;

            LinearSpeed += Gas / 256.0f * CTimer::GetTimeStep() * 0.002f / float(numCarriagesPulled);

            if (Brake != 0.0f) {
                float fTrainSpeed = LinearSpeed;
                if (LinearSpeed < 0.0f) {
                    fTrainSpeed = -fTrainSpeed;
                }
                float fBreak = Brake / 256.0f * CTimer::GetTimeStep() * 0.006f / float(numCarriagesPulled);
                if (fTrainSpeed >= fBreak) {
                    if (LinearSpeed < 0.0f) {
                        LinearSpeed += fBreak;
                    } else {
                        LinearSpeed -= fBreak;
                    }
                } else {
                    LinearSpeed = 0.0f;
                }
            }

            LinearSpeed *= pow(0.999750018119812f, CTimer::GetTimeStep());
            if (!m_nTrainFlags.bDirection) {
                LinearSpeed = -LinearSpeed;
            }

            PositionOnTrack += CTimer::GetTimeStep() * LinearSpeed;

            if (GetStatus() == STATUS_PLAYER) {

                float fTheTrainSpeed = LinearSpeed;
                if (fTheTrainSpeed < 0.0f) {
                    fTheTrainSpeed = -fTheTrainSpeed;
                }
                if (fTheTrainSpeed > 1.0f * 0.95f) {
                    CPad::GetPad()->StartShake(300, 70, 0);
                    TheCamera.CamShake(0.1f, GetPosition());
                }

                fTheTrainSpeed = LinearSpeed;
                if (fTheTrainSpeed < 0.0f) {
                    fTheTrainSpeed = -fTheTrainSpeed;
                }

                if (fTheTrainSpeed > 1.0f) {
                    int32 nNodeIndex = CurrentNode;
                    int32 previousNodeIndex = nNodeIndex - 1;
                    if (previousNodeIndex < 0) {
                        previousNodeIndex = numTrackNodes;
                    }

                    int32 previousNodeIndex2 = previousNodeIndex - 1;
                    if (previousNodeIndex2 < 0) {
                        previousNodeIndex2 = numTrackNodes;
                    }

                    CTrainNode* pCurrentTrainNode = &trainNodes[CurrentNode];
                    CTrainNode* pPreviousTrainNode = &trainNodes[previousNodeIndex];
                    CTrainNode* pPreviousTrainNode2 = &trainNodes[previousNodeIndex2];

                    CVector vecDifference1 = pCurrentTrainNode->GetPosn() - pPreviousTrainNode->GetPosn();
                    CVector vecDifference2 = pPreviousTrainNode->GetPosn() - pPreviousTrainNode2->GetPosn();
                    vecDifference1.Normalise();
                    vecDifference2.Normalise();

                    if (DotProduct(vecDifference1, vecDifference2) < 0.996f) {
                        CTrain* carriage = this;
                        bool bIsInTunnel = false;
                        while (!bIsInTunnel) {
                            bIsInTunnel = carriage->IsInTunnel();
                            carriage = carriage->pLinkedToBackward;
                            if (!carriage) {
                                if (!bIsInTunnel) {
                                    CTrain* theTrainCarriage = this;
                                    do {
                                        m_nTrainFlags.bDerailed = true;
                                        theTrainCarriage->m_nPhysicalFlags.bInfiniteMass = false;
                                        theTrainCarriage->m_nPhysicalFlags.bDontProcessCollisionOurSelves = false;
                                        theTrainCarriage->SetIsStatic(false);
                                        theTrainCarriage = theTrainCarriage->pLinkedToBackward;
                                    } while (theTrainCarriage);

                                    CPhysical::ProcessControl();
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (PositionOnTrack < 0.0f) {
            do {
                PositionOnTrack += fTotalTrackLength;
            } while (PositionOnTrack < 0.0f);
        }

        if (PositionOnTrack >= fTotalTrackLength) {
            do {
                PositionOnTrack -= fTotalTrackLength;
            } while (PositionOnTrack >= fTotalTrackLength);
        }

        float fNextNodeTrackLength = 0.0f;
        int32 nextNodeIndex = CurrentNode + 1;
        if (nextNodeIndex < numTrackNodes) {
            CTrainNode* nextTrainNode = &trainNodes[nextNodeIndex];
            fNextNodeTrackLength = nextTrainNode->GetDistanceFromStart();
        } else {
            fNextNodeTrackLength = fTotalTrackLength;
            nextNodeIndex = 0;
        }

        CTrainNode* theTrainNode = &trainNodes[CurrentNode];
        float fCurrentNodeTrackLength = theTrainNode->GetDistanceFromStart();
        while (PositionOnTrack < fCurrentNodeTrackLength || fNextNodeTrackLength < PositionOnTrack) {
            int32 newNodeIndex = CurrentNode - 1; // previous node
            if (fCurrentNodeTrackLength <= PositionOnTrack) {
                newNodeIndex = CurrentNode + 1; // next node
            }
            CurrentNode = newNodeIndex % numTrackNodes;
            m_vehicleAudio.AddAudioEvent(AE_TRAIN_CLACK, 0.0f);

            theTrainNode = &trainNodes[CurrentNode];
            fCurrentNodeTrackLength = theTrainNode->GetDistanceFromStart();

            nextNodeIndex = CurrentNode + 1;
            if (nextNodeIndex < numTrackNodes) {
                CTrainNode* nextTrainNode = &trainNodes[nextNodeIndex];
                fNextNodeTrackLength = nextTrainNode->GetDistanceFromStart();
            } else {
                fNextNodeTrackLength = fTotalTrackLength;
                nextNodeIndex = 0;
            }
        }

        CTrainNode* nextTrainNode = &trainNodes[nextNodeIndex];
        fNextNodeTrackLength = nextTrainNode->GetDistanceFromStart();

        float fTrackNodeDifference = fNextNodeTrackLength - fCurrentNodeTrackLength;
        if (fTrackNodeDifference < 0.0f) {
            fTrackNodeDifference += fTotalTrackLength;
        }

        float fTheDistance = (PositionOnTrack - fCurrentNodeTrackLength) / fTrackNodeDifference;
        CVector vecPosition1 = theTrainNode->GetPosn() * (1.0f - fTheDistance) + nextTrainNode->GetPosn() * fTheDistance;

        CColModel* vehicleColModel = CModelInfo::GetModelInfo(m_nModelIndex)->GetColModel();
        const CBoundingBox& bbox = vehicleColModel->GetBoundingBox();
        float fTotalCurrentRailDistance = bbox.GetLength() + PositionOnTrack;
        if (fTotalCurrentRailDistance > fTotalTrackLength) {
            fTotalCurrentRailDistance -= fTotalTrackLength;
        }

        nextNodeIndex = CurrentNode + 1;
        if (nextNodeIndex < numTrackNodes) {
            fNextNodeTrackLength = trainNodes[nextNodeIndex].GetDistanceFromStart();
        } else {
            fNextNodeTrackLength = fTotalTrackLength;
            nextNodeIndex = 0;
        }

        int32 trainNodeIndex = CurrentNode;
        while (fTotalCurrentRailDistance < fCurrentNodeTrackLength || fTotalCurrentRailDistance > fNextNodeTrackLength) {
            trainNodeIndex = (trainNodeIndex + 1) % numTrackNodes;

            theTrainNode = &trainNodes[trainNodeIndex];
            fCurrentNodeTrackLength = theTrainNode->GetDistanceFromStart();

            nextNodeIndex = trainNodeIndex + 1;
            if (nextNodeIndex < numTrackNodes) {
                fNextNodeTrackLength = trainNodes[nextNodeIndex].GetDistanceFromStart();
            } else {
                fNextNodeTrackLength = fTotalTrackLength;
                nextNodeIndex = 0;
            }
        }

        fNextNodeTrackLength = trainNodes[nextNodeIndex].GetDistanceFromStart();

        fTrackNodeDifference = fNextNodeTrackLength - fCurrentNodeTrackLength;
        if (fTrackNodeDifference < 0.0f) {
            fTrackNodeDifference += fTotalTrackLength;
        }

        fTheDistance = (fTotalCurrentRailDistance - fCurrentNodeTrackLength) / fTrackNodeDifference;
        CVector vecPosition2 = theTrainNode->GetPosn() * (1.0f - fTheDistance) + trainNodes[nextNodeIndex].GetPosn() * fTheDistance;

        {
            CVector& vecVehiclePosition = GetPosition();
            vecVehiclePosition = (vecPosition1 + vecPosition2) / 2.0f;
            vecVehiclePosition.z += m_pHandlingData->m_fSuspensionLowerLimit - bbox.m_vecMin.z;
        }

        GetForward() = vecPosition2 - vecPosition1;
        GetForward().Normalise();
        if (!m_nTrainFlags.bDirection) {
            GetForward() *= -1.0f;
        }

        CVector vecTemp(0.0f, 0.0f, 1.0f);
        CrossProduct(&GetRight(), &GetForward(), &vecTemp);
        GetRight().Normalise();
        CrossProduct(&GetUp(), &GetRight(), &GetForward());

        auto fTrainNodeLighting     = theTrainNode->GetLightingFromCollision().GetCurrentLighting();
        auto fTrainNextNodeLighting = trainNodes[nextNodeIndex].GetLightingFromCollision().GetCurrentLighting();

        fTrainNodeLighting += (fTrainNextNodeLighting - fTrainNodeLighting) * fTheDistance;
        m_fContactSurfaceBrightness = fTrainNodeLighting;
        m_vecMoveSpeed = (1.0f / CTimer::GetTimeStep()) * (GetPosition() - vecOldTrainPosition);

        float fNewTrainHeading = GetHeading();
        float fHeading = fNewTrainHeading - fOldTrainHeading;
        if (fHeading <= PI) {
            if (fHeading < -PI) {
                fHeading += TWO_PI;
            }
        } else {
            fHeading -= TWO_PI;
        }

        m_vecTurnSpeed = CVector(0.0f, 0.0f, fHeading / CTimer::GetTimeStep());

        if (m_nTrainFlags.bDerailed) {
            m_vecMoveSpeed *= -1.0f;
            m_vecTurnSpeed *= -1.0f;

            ApplyMoveSpeed();

            m_vecMoveSpeed *= -1.0f;
            m_vecTurnSpeed *= -1.0f;

            CPhysical::ProcessControl();
        } else {
            m_vecMoveSpeed.x = std::clamp(m_vecMoveSpeed.x, -2.0f, 2.0f);
            m_vecMoveSpeed.y = std::clamp(m_vecMoveSpeed.y, -2.0f, 2.0f);
            m_vecMoveSpeed.z = std::clamp(m_vecMoveSpeed.z, -2.0f, 2.0f);

            m_vecTurnSpeed.x = std::clamp(m_vecTurnSpeed.x, -0.1f, 0.1f);
            m_vecTurnSpeed.y = std::clamp(m_vecTurnSpeed.y, -0.1f, 0.1f);
            m_vecTurnSpeed.z = std::clamp(m_vecTurnSpeed.z, -0.1f, 0.1f);
        }

        UpdateRwMatrix();
        UpdateRwFrame();
        RemoveAndAdd();

        SetIsStuck(false);
        SetWasPostponed(false);
        SetIsInSafePosition(true);

        m_fMovingSpeed = DistanceBetweenPoints(GetPosition(), vecOldTrainPosition);

        if (m_nTrainFlags.bEngine || m_nTrainFlags.bCaboose) {
            CVector vecPoint = bbox.m_vecMax.y * GetForward();
            vecPoint += GetPosition();
            vecPoint += CTimer::GetTimeStep() * m_vecMoveSpeed;

            MarkSurroundingEntitiesForCollisionWithTrain(vecPoint, 3.0f, this, false);
        }

        if (!vehicleFlags.bWarnedPeds) {
            CCarCtrl::ScanForPedDanger(this);
        }
        return;
    } else {
        if (!GetIsStuck()) {
            float fMaxForce = 0.003f;
            float fMaxTorque = 0.0009f;
            float fMaxMovingSpeed = 0.005f;

            if (GetStatus() != STATUS_PLAYER) {
                fMaxForce = 0.006f;
                fMaxTorque = 0.0015f;
                fMaxMovingSpeed = 0.015f;
            }

            float fMaxForceTimeStep  = (fMaxForce  * CTimer::GetTimeStep()) * (fMaxForce * CTimer::GetTimeStep());
            float fMaxTorqueTimeStep = (fMaxTorque * CTimer::GetTimeStep()) * (fMaxTorque * CTimer::GetTimeStep());

            m_vecForce  = (m_vecForce  + m_vecMoveSpeed) / 2.0f;
            m_vecTorque = (m_vecTorque + m_vecTurnSpeed) / 2.0f;

            if (m_vecForce.SquaredMagnitude() > fMaxForceTimeStep ||
                m_vecTorque.SquaredMagnitude() > fMaxTorqueTimeStep ||
                m_fMovingSpeed >= fMaxMovingSpeed ||
                m_fDamageIntensity > 0.0f && m_pDamageEntity != nullptr && m_pDamageEntity->GetIsTypePed()
            ) {
                m_nFakePhysics = 0;
            } else {
                m_nFakePhysics += 1;
                if (m_nFakePhysics > 10 /*&& !plugin::Call<0x424100>()*/) {
                    // if (m_nFakePhysics > 10) { // OG redundant check
                        m_nFakePhysics = 10;
                    // }

                    ResetMoveSpeed();
                    ResetTurnSpeed();
                    SkipPhysics();
                    return;
                }
            }
        }

        CPhysical::ProcessControl();

        CVector vecMoveForce{}, vecTurnForce{};
        if (mod_Buoyancy.ProcessBuoyancy(this, m_fBuoyancyConstant, &vecMoveForce, &vecTurnForce)) {
            m_nPhysicalFlags.bForceFullWaterCheck = true;

            float fTimeStep = 0.01f;
            if (CTimer::GetTimeStep() >= 0.01f) {
                fTimeStep = CTimer::GetTimeStep();
            }

            float fSpeedFactor = 1.0f - vecOldTrainPosition.z / (fTimeStep * m_fMass * 0.008f) * 0.05f;
            fSpeedFactor = std::pow(fSpeedFactor, CTimer::GetTimeStep());

            m_vecMoveSpeed *= fSpeedFactor;
            m_vecTurnSpeed *= fSpeedFactor;
            ApplyMoveForce(vecOldTrainPosition.x, vecOldTrainPosition.y, vecOldTrainPosition.z);
            ApplyTurnForce(vecTurnForce, vecMoveForce);
        }
    }
}

