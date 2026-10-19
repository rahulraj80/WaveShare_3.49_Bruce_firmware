#include "power_commands.h"
#include "core/settings.h"
#include <globals.h>

uint32_t poweroffCallback(cmd *c) {
    powerOff();
    esp_deep_sleep_start(); // only wake up via hardware reset
    return true;
}

uint32_t rebootCallback(cmd *c) {
    ESP.restart();
    return true;
}

uint32_t sleepCallback(cmd *c) {
    setSleepMode();
    return true;
}

// Launcher's bootloader starts the Launcher after a deep-sleep wake (and the
// installed app after a soft restart), so a short sleep hands control back.
// Without the Launcher this is just a reboot.
uint32_t launcherCallback(cmd *c) {
    Serial.println("Rebooting to the Launcher...");
    delay(100);
    esp_sleep_enable_timer_wakeup(100 * 1000);
    esp_deep_sleep_start();
    return true;
}

void createPoweroffCommand(SimpleCLI *cli) { Command cmd = cli->addCommand("poweroff", poweroffCallback); }

void createRebootCommand(SimpleCLI *cli) { Command cmd = cli->addCommand("reboot", rebootCallback); }

void createSleepCommand(SimpleCLI *cli) { Command cmd = cli->addCommand("sleep", sleepCallback); }

void createPowerCommand(SimpleCLI *cli) {
    Command cmd = cli->addCompositeCommand("power");

    Command cmdOff = cmd.addCommand("off", poweroffCallback);
    Command cmdReboot = cmd.addCommand("reboot", rebootCallback);
    Command cmdSleep = cmd.addCommand("sleep", sleepCallback);
}

void createPowerCommands(SimpleCLI *cli) {
    createPoweroffCommand(cli);
    createRebootCommand(cli);
    createSleepCommand(cli);
    cli->addCommand("launcher", launcherCallback);

    createPowerCommand(cli);
}
