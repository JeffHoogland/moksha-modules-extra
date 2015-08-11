#include <e.h>
#include "e_mod_main.h"

void
_mem_get_values (Config_Item * ci, int *real, int *swap, int *total_real,
		 int *total_swap)
{
  FILE *pmeminfo = NULL;
  int cursor = 0;
  char *line, *field;
  unsigned char c;
  long int value = 0, mtotal = 0, stotal = 0, mfree = 0, sfree = 0;
  ldiv_t ldresult;
  long int liresult;

  if (!(pmeminfo = fopen ("/proc/meminfo", "r")))
    {
      fprintf (stderr, "can't open /proc/meminfo");
      return;
    }

  line = (char *) calloc (64, sizeof (char));
  while (fscanf (pmeminfo, "%c", &c) != EOF)
    {
      if (c != '\n')
	line[cursor++] = c;
      else
	{
	  field = (char *) malloc (strlen (line) * sizeof (char));
	  sscanf (line, "%s %ld kB", field, &value);
	  if (!strcmp (field, "MemTotal:"))
	    mtotal = value;
	  else if (!strcmp (field, "MemFree:"))
	    mfree = value;
	  else if (ci->real_ignore_buffers && (!strcmp (field, "Buffers:")))
	    mfree += value;
	  else if (ci->real_ignore_cached && (!strcmp (field, "Cached:")))
	    mfree += value;
	  else if (ci->real_ignore_cached && (!strcmp (field, "SwapCached:")))
	    sfree += value;
	  else if (!strcmp (field, "SwapTotal:"))
	    stotal = value;
	  else if (!strcmp (field, "SwapFree:"))
	    sfree = value;

	  free (line);
	  free (field);
	  cursor = 0;
	  line = (char *) calloc (64, sizeof (char));
	}
    }
  fclose (pmeminfo);

  if (stotal >= 1)
    {
      ldresult = ldiv (stotal, 100);
      liresult = ldresult.quot;
      ldresult = ldiv ((stotal - sfree), liresult);
    }

  ldresult = ldiv (mtotal, 100);
  liresult = ldresult.quot;
  ldresult = ldiv ((mtotal - mfree), liresult);

  *real = (mtotal - mfree);
  *swap = (stotal - sfree);
  *total_real = mtotal;
  *total_swap = stotal;
}
