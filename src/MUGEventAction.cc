#include "MUGEventAction.hh"

#include <sstream>
#include <chrono>

#include "G4RunManager.hh"
#include "MUGRun.hh"

#include "G4GenericMessenger.hh"
#include "MUGManager.hh"
#include "MUGRunAction.hh"
#include "MUGUserAction.hh"
#include "MUGLog.hh"

#include "fmt/chrono.h"

MUGEventAction::MUGEventAction() {
  this->DefineCommands();
}

void MUGEventAction::BeginOfEventAction(const G4Event* event) {

  auto print_modulo = MUGManager::GetMUGManager()->GetPrintModulo();
  if ((event->GetEventID()+1) % print_modulo == 0) {

    auto current_run = dynamic_cast<const MUGRun*>(G4RunManager::GetRunManager()->GetCurrentRun());
    auto tot_events = current_run->GetNumberOfEventToBeProcessed();

    auto start_time = current_run->GetStartTime();
    auto time_now = std::chrono::system_clock::now();
    auto tot_elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(time_now - start_time).count();
    long partial = 0;
    long elapsed_d = (tot_elapsed_s - partial) / 86400; partial += elapsed_d * 86400;
    long elapsed_h = (tot_elapsed_s - partial) / 3600;  partial += elapsed_h * 3600;
    long elapsed_m = (tot_elapsed_s - partial) / 60;    partial += elapsed_m * 60;
    long elapsed_s = tot_elapsed_s - partial;

    // FIXME: how to cast the percentage to int?
    MUGLog::OutFormat(MUGLog::summary, "Processing event nr. {:d} ({:.0f}%), at {:d} days, {:d} hours, {:d} minutes and {} seconds",
        event->GetEventID()+1, (event->GetEventID()+1)*100./tot_events,
        elapsed_d, elapsed_h, elapsed_m, elapsed_s);
  }
}

void MUGEventAction::EndOfEventAction(const G4Event* /*event*/) {}

void MUGEventAction::DefineCommands() {

  fMessenger = std::make_unique<G4GenericMessenger>(this, "/MUG/Output/",
      "Commands for controlling the event actions");
}

// vim: tabstop=2 shiftwidth=2 expandtab
