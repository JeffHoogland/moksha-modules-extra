
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_EEZE

#include <e.h>
#include "e_mod_places.h"
#include <Eeze.h>
#include <Eeze_Disk.h>

/* Local Function Prototypes */


/* Local Variables */

// Eina_Bool eeze_mount_mtab_scan (void)
// Eina_Bool eeze_mount_fstab_scan (void)

// Eina_Bool eeze_mount_tabs_watch (void)
// void      eeze_mount_tabs_unwatch (void)

// Eina_Bool eeze_disk_mount (Eeze_Disk *disk)
// Eina_Bool eeze_disk_unmount (Eeze_Disk *disk)
// Eina_Bool eeze_disk_eject (Eeze_Disk *disk)

/* Implementation */

void _places_eeze_disk_print(Eeze_Disk *disk)
{
   eeze_disk_scan(disk); // NEEDED ??

   printf("PLACES: SysPath: %s\n", eeze_disk_syspath_get(disk));
   printf("PLACES:        devpath: %s\n", eeze_disk_devpath_get(disk));
   printf("PLACES:    mount_point: %s\n", eeze_disk_mount_point_get(disk));
   printf("PLACES:      mountopts: %ld\n", eeze_disk_mountopts_get(disk));
   printf("PLACES:          label: %s\n", eeze_disk_label_get(disk));
   printf("PLACES:           type: %d\n", eeze_disk_type_get(disk));
   printf("PLACES:         fstype: %s\n", eeze_disk_fstype_get(disk));
   printf("PLACES:        mounted: %d\n", eeze_disk_mounted_get(disk));
   printf("PLACES:      removable: %d\n", eeze_disk_removable_get(disk));
   printf("PLACES:  mount_wrapper: %s\n", eeze_disk_mount_wrapper_get(disk));
   
   printf("PLACES:         vendor: %s\n", eeze_disk_vendor_get(disk));
   printf("PLACES:          model: %s\n", eeze_disk_model_get(disk));
   printf("PLACES:         serial: %s\n", eeze_disk_serial_get(disk));
   printf("PLACES:           uuid: %s\n", eeze_disk_uuid_get(disk));

   printf("PLACES: ---------------\n");
}

Eina_Bool
places_eeze_init(void)
{
   printf("PLACES: eeze: init()\n");

   // This will fail on compilation/runtime if Disk functions not available
   eeze_disk_function();

   // init eeze
   if (!eeze_init())
   {
      printf("Impossible to setup eeze.\n");
      return EINA_FALSE;
   }

   // check eeze mount/umont/eject support
   if (!eeze_disk_can_mount() ||
       !eeze_disk_can_unmount() ||
       !eeze_disk_can_eject())
   {
      printf("Eeze don't have mount/umount/eject support.\n");
      eeze_shutdown();
      return EINA_FALSE;
   }

   // if (!eeze_mount_tabs_watch())
   // {
      // printf("Cannot setup watcher for fstab/mtab.\n");
      // eeze_shutdown();
      // return EINA_FALSE;
   // }

   eeze_mount_fstab_scan();
   eeze_mount_mtab_scan();


   Eina_List *disks;
   const char *syspath;
   Eeze_Disk *disk;

   
   disks = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   printf("PLACES: Found the following internal disks:\n");
   EINA_LIST_FREE(disks, syspath)
     {
        disk = eeze_disk_new(syspath);
        _places_eeze_disk_print(disk);
        eina_stringshare_del(syspath);
        eeze_disk_free(disk);
     }

   disks = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_MOUNTABLE, NULL);
   printf("PLACES: Found the following mountable disks:\n");
   EINA_LIST_FREE(disks, syspath)
     {
        disk = eeze_disk_new(syspath);
        _places_eeze_disk_print(disk);
        eeze_disk_free(disk);
        eina_stringshare_del(syspath);
     }

   disks = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_CDROM, NULL);
   printf("PLACES: Found the following cdrom disks:\n");
   EINA_LIST_FREE(disks, syspath)
     {
        disk = eeze_disk_new(syspath);
        _places_eeze_disk_print(disk);
        
        eeze_disk_free(disk);
        eina_stringshare_del(syspath);
     }

   // disks = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, NULL);
   // printf("PLACES: Found the following removable disks:\n");
   // EINA_LIST_FREE(disks, syspath)
     // {
        // places_eeze_print_syspath_info(syspath);
        // eina_stringshare_del(syspath);
     // }

   printf("PLACES: Searching for your root:\n");
   disk = eeze_disk_new_from_mount("/");
   if (disk)
   {
      _places_eeze_disk_print(disk);
      eeze_disk_free(disk);
   }

   return EINA_TRUE;
}

void
places_eeze_shutdown(void)
{
   eeze_mount_tabs_unwatch();
   eeze_shutdown();
}


/* Internals */

#endif
