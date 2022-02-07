#include "MUGNavigationTools.hh"

#include <set>
#include <map>

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4Material.hh"

#include "MUGLog.hh"

#include "fmt/core.h"

G4VPhysicalVolume* MUGNavigationTools::FindPhysicalVolume(std::string name) {
    auto const& store = *G4PhysicalVolumeStore::GetInstance();
    auto result = std::find_if(store.begin(), store.end(), [&name](auto v) { return std::string(v->GetName()) == name; });
    if (result == store.end()) {
      MUGLog::Out(MUGLog::error, "Physical volume ", name, " not found in store. Returning nullptr");
      return nullptr;
    }
    return *result;
}

G4LogicalVolume* MUGNavigationTools::FindLogicalVolume(std::string name) {
    auto const& store = *G4LogicalVolumeStore::GetInstance();
    auto result = std::find_if(store.begin(), store.end(), [&name](auto v) { return std::string(v->GetName()) == name; });
    if (result == store.end()) {
      MUGLog::Out(MUGLog::error, "Logical volume ", name, " not found in store. Returning nullptr");
      return nullptr;
    }
    return *result;
}

G4VPhysicalVolume* MUGNavigationTools::FindDirectMother(G4VPhysicalVolume* volume) {

  std::set<G4VPhysicalVolume*> ancestors;
  for (const auto& v : *G4PhysicalVolumeStore::GetInstance()) {
    if (v->GetLogicalVolume()->IsAncestor(volume)) ancestors.insert(v);
  }

  if (ancestors.empty()) {
    MUGLog::Out(MUGLog::error, "No ancestors found for physical volume '",
        volume->GetName(), "', returning nullptr");
  }

  // check elements one-by-one
  for (auto it = ancestors.begin(); it != ancestors.end(); ) {
    // determine if element should be erased or not because it is an ancestor
    // of someone else in the list
    bool to_erase = false;
    for (auto itt = ancestors.begin(); itt != ancestors.end(); ) {
      if (*it != *itt and (*it)->GetLogicalVolume()->IsAncestor(*itt)) {
        to_erase = true;
        break;
      }
      else itt++;
    }
    if (to_erase) it = ancestors.erase(it);
    else it++;
  }

  if (ancestors.size() != 1) {
    MUGLog::Out(MUGLog::error, "Could not find a unique direct mother volume, ",
        "this cannot be! Returning first ancestor in the list");
  }

  return *ancestors.begin();
}

void MUGNavigationTools::PrintListOfLogicalVolumes() {

  int max_length = 0;
  // use std::set to have volumes automatically sorted by name
  std::set<std::pair<std::string, std::string>> volumes;
  for (const auto& v : *G4LogicalVolumeStore::GetInstance()) {

    if (v->GetName().size() > std::size_t(max_length)) max_length = v->GetName().size();

    volumes.insert({v->GetName(),
        fmt::format("{} daugh. // {} // {} // {} // {} // {}",
            v->GetNoDaughters(),
            std::string(G4BestUnit(v->GetMaterial()->GetDensity(), "Volumic Mass")),
            std::string(G4BestUnit(v->GetMass(), "Mass")),
            std::string(G4BestUnit(v->GetMass() / v->GetMaterial()->GetDensity(), "Volume")),
            std::string(G4BestUnit(v->GetMaterial()->GetPressure(), "Pressure")),
            std::string(G4BestUnit(v->GetMaterial()->GetTemperature(), "Temperature")))
        });
  }

  MUGLog::Out(MUGLog::summary, "Logical volumes registered in the volume store (alphabetic order):");
  for (const auto& v : volumes) {
    MUGLog::OutFormat(MUGLog::summary, " · {:<" + std::to_string(max_length) + "} // {}", v.first, v.second);
  }
  MUGLog::Out(MUGLog::summary, "");
  MUGLog::Out(MUGLog::summary, "Total: ", volumes.size(), " volumes");
}

void MUGNavigationTools::PrintListOfPhysicalVolumes() {

  std::vector<std::string> volumes;
  for (const auto& v : *G4PhysicalVolumeStore::GetInstance()) {
    volumes.push_back(" · " + v->GetName() + " (" + std::to_string(v->GetCopyNo()) +
        + ") // from logical: " + v->GetLogicalVolume()->GetName());
  }
  std::sort(volumes.begin(), volumes.end());

  MUGLog::Out(MUGLog::summary, "Physical volumes registered in the volume store in alphabetic order [name (copy nr.)]:");
  for (const auto& v : volumes) MUGLog::Out(MUGLog::summary, v);
  MUGLog::Out(MUGLog::summary, "");
  MUGLog::Out(MUGLog::summary, "Total: ", volumes.size(), " volumes");
}

// vim: tabstop=2 shiftwidth=2 expandtab
