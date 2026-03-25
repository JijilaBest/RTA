#include <stdio.h>
#include "nvs_flash.h"
#include "active_look.hpp"
#include "ble_manager.hpp"

// Instance globale permettant de piloter l'affichage des lunettes.
// Les autres composants (comme ble_manager) pourront l'utiliser.
ActiveLook myGlasses;

// Point d'entrée principal du programme pour ESP-IDF
extern "C" void app_main(void) {
    // Étape 1 : Initialisation de la mémoire NVS (Non-Volatile Storage)
    // Cela est indispensable car la pile Bluetooth (NimBLE) l'utilise pour stocker 
    // ses identifiants et clés de sécurité internes.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase(); // Si la partition complète ou corrompue, on efface et réessaie
        nvs_flash_init();
    }

    // Étape 2 : Initialisation et démarrage du Bluetooth
    // Cette fonction lancera un fil d'exécution séparé qui va scanner
    // l'environnement à la recherche des lunettes "ENGO" pour s'y connecter.
    ble_manager_init();
    
  
    // gps_manager_init();
}