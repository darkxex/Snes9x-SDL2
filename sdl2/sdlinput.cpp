/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  See CREDITS file to find the copyright owners of this file.

  SDL Input/Audio/Video code (many lines of code come from snes9x & drnoksnes)
  The code conversion to SDL2 was done by https://github.com/darkxex/
  (c) Copyright 2011         Makoto Sugano (makoto.sugano@gmail.com)

  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/

#include "sdl_snes9x.h"

#include "snes9x.h"
#include "port.h"
#include "controls.h"

using namespace std;
std::map <string, int> name_sdlkeysym;

int mynumid;

ConfigFile::secvec_t	keymaps;

s9xcommand_t S9xInitCommandT (const char *n)
{
	s9xcommand_t	cmd;

	cmd.type         = S9xBadMapping;
	cmd.multi_press  = 0;
	cmd.button_norpt = 0;
	cmd.port[0]      = 0xff;
	cmd.port[1]      = 0;
	cmd.port[2]      = 0;
	cmd.port[3]      = 0;

	return (cmd);
}

char * S9xGetDisplayCommandName (s9xcommand_t cmd)
{
	return (strdup("None"));
}

void S9xHandleDisplayCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}

// domaemon: 2) here we send the keymapping request to the SNES9X
// domaemon: MapInput (J, K, M)
bool8 S9xMapInput (const char *n, s9xcommand_t *cmd)
{
	int	i, j, d;
	char	*c;

	// domaemon: linking PseudoPointer# and command
	if (!strncmp(n, "PseudoPointer", 13))
	{
		if (n[13] >= '1' && n[13] <= '8' && n[14] == '\0')
		{
			return (S9xMapPointer(PseudoPointerBase + (n[13] - '1'), *cmd, false));
		}
		else
		{
			goto unrecog;
		}
	}

	// domaemon: linking PseudoButton# and command
	if (!strncmp(n, "PseudoButton", 12))
	{
		if (isdigit(n[12]) && (j = strtol(n + 12, &c, 10)) < 256 && (c == NULL || *c == '\0'))
		{
			return (S9xMapButton(PseudoButtonBase + j, *cmd, false));
		}
		else
		{
			goto unrecog;
		}
	}

	if (!(isdigit(n[1]) && isdigit(n[2]) && n[3] == ':'))
		goto unrecog;

	switch (n[0])
	{
		case 'J': // domaemon: joysticks input mapping
		{
			d = ((n[1] - '0') * 10 + (n[2] - '0')) << 24;
			d |= 0x80000000;
			i = 4;
			
			if (!strncmp(n + i, "Axis", 4))	// domaemon: joystick axis
			{
				d |= 0x8000; // Axis mode
				i += 4;
			}
			else if (n[i] == 'B') // domaemon: joystick button
			{	
				i++;
			}
			else
			{
				goto unrecog;
			}
			
			d |= j = strtol(n + i, &c, 10); // Axis or Button id
			if ((c != NULL && *c != '\0') || j > 0x3fff)
				goto unrecog;
			
			if (d & 0x8000)
				return (S9xMapAxis(d, *cmd, false));
			
			return (S9xMapButton(d, *cmd, false));
		}

		case 'K':
		{
			d = 0x00000000;
			
			for (i = 4; n[i] != '\0' && n[i] != '+'; i++) ;
			
			if (n[i] == '\0' || i == 4) {
				// domaemon: if no mod keys are found.
				i = 4;
			}
			else
			{
				// domaemon: mod keys are not supported now.
				goto unrecog;
			}

			string keyname (n + i); // domaemon: SDL_keysym in string format.
			
			d |= name_sdlkeysym[keyname];
			return (S9xMapButton(d, *cmd, false));
		}

		case 'M':
		{
			d = 0x40000000;

			if (!strncmp(n + 4, "Pointer", 7))
			{
				d |= 0x8000;
				
				if (n[11] == '\0')
					return (S9xMapPointer(d, *cmd, true));
				
				i = 11;
			}
			else if (n[4] == 'B')
			{
				i = 5;
			}
			else
			{
				goto unrecog;
			}
			
			d |= j = strtol(n + i, &c, 10);
			
			if ((c != NULL && *c != '\0') || j > 0x7fff)
				goto unrecog;
			
			if (d & 0x8000)
				return (S9xMapPointer(d, *cmd, true));

			return (S9xMapButton(d, *cmd, false));
		}
	
		default:
			break;
	}

unrecog:
	char	*err = new char[strlen(n) + 34];

	sprintf(err, "Unrecognized input device name '%s'", n);
	perror(err);
	delete [] err;

	return (false);
}

// domaemon: SetupDefaultKeymap -> MapInput (JS) -> MapDisplayInput (KB)
void S9xSetupDefaultKeymap (void)
{
	

	S9xUnmapAllControls();
    S9xMapButton(SDLK_RIGHT,  S9xGetCommandT("Joypad1 Right"), false);
   S9xMapButton(SDLK_LEFT,  S9xGetCommandT("Joypad1 Left"), false);
     S9xMapButton(SDLK_DOWN,S9xGetCommandT("Joypad1 Down"), false);
     S9xMapButton(SDLK_UP, S9xGetCommandT("Joypad1 Up"), false);
     S9xMapButton(SDLK_RETURN, S9xGetCommandT("Joypad1 Start"), false);
     S9xMapButton(SDLK_SPACE,  S9xGetCommandT("Joypad1 Select"), false);
     S9xMapButton(SDLK_d,S9xGetCommandT("Joypad1 A"), false);
     S9xMapButton(SDLK_c, S9xGetCommandT("Joypad1 B"), false);
     S9xMapButton(SDLK_s, S9xGetCommandT("Joypad1 X"), false);
     S9xMapButton(SDLK_x, S9xGetCommandT("Joypad1 Y"), false);
     S9xMapButton(SDLK_a, S9xGetCommandT("Joypad1 L"), false);
     S9xMapButton(SDLK_z, S9xGetCommandT("Joypad1 R"), false);
	

}

// domaemon: FIXME, just collecting the essentials.
// domaemon: *) here we define the keymapping.
void S9xParseInputConfig (ConfigFile &conf, int pass)
{
	
	

	return;
}

void S9xInitInputDevices (void)
{
	SDL_Joystick * joystick[4] = {NULL, NULL, NULL, NULL};

	// domaemon: 1) initializing the joystic subsystem
	SDL_InitSubSystem (SDL_INIT_JOYSTICK);

	/*
	 * domaemon: 2) check how may joysticks are connected
	 * domaemon: 3) relate paddev1 to SDL_Joystick[0], paddev2 to SDL_Joystick[1]...
	 * domaemon: 4) print out the joystick name and capabilities
	 */

	int num_joysticks = SDL_NumJoysticks();

	if (num_joysticks == 0)
	{
		fprintf(stderr, "joystick: No joystick found.\n");
	}
	else
	{
		SDL_JoystickEventState (SDL_ENABLE);

		// domaemon: FIXME should check if num_joysticks is below 4..
		for (int i = 0; i < num_joysticks; i++)
		{
			joystick[i] = SDL_JoystickOpen (i);
			printf ("  %d-axis %d-buttons %d-balls %d-hats \n",
				SDL_JoystickNumAxes(joystick[i]),
				SDL_JoystickNumButtons(joystick[i]),
				SDL_JoystickNumBalls(joystick[i]),
				SDL_JoystickNumHats(joystick[i]));
		}
	}
}

void S9xProcessEvents (bool8 block)
{
	SDL_Event event;
	bool8 quit_state = FALSE;

	while ((block) || (SDL_PollEvent (&event) != 0))
	{
		switch (event.type) {
		case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_RETURN)
		S9xReportButton(SDLK_RETURN, true );

		if(event.key.keysym.sym == SDLK_RIGHT)
		S9xReportButton(SDLK_RIGHT, true );

		if(event.key.keysym.sym == SDLK_LEFT)
		S9xReportButton(SDLK_LEFT, true );

		if(event.key.keysym.sym == SDLK_DOWN)
		S9xReportButton(SDLK_DOWN, true );

		if(event.key.keysym.sym == SDLK_UP)
		S9xReportButton(SDLK_UP, true );

		if(event.key.keysym.sym == SDLK_SPACE)
		S9xReportButton(SDLK_SPACE, true );

		if(event.key.keysym.sym == SDLK_d)
		S9xReportButton(SDLK_d, true );
		
		if(event.key.keysym.sym == SDLK_c)
		S9xReportButton(SDLK_c, true );

		if(event.key.keysym.sym == SDLK_s)
		S9xReportButton(SDLK_s, true );

		if(event.key.keysym.sym == SDLK_x)
		S9xReportButton(SDLK_x, true );

		if(event.key.keysym.sym == SDLK_a)
		S9xReportButton(SDLK_a, true );

		if(event.key.keysym.sym == SDLK_z)
		S9xReportButton(SDLK_z, true );
        break;

		case SDL_KEYUP:		
						
				 if(event.key.keysym.sym == SDLK_RETURN)
		S9xReportButton(SDLK_RETURN, false );

		if(event.key.keysym.sym == SDLK_RIGHT)
		S9xReportButton(SDLK_RIGHT, false );

		if(event.key.keysym.sym == SDLK_LEFT)
		S9xReportButton(SDLK_LEFT, false);

		if(event.key.keysym.sym == SDLK_DOWN)
		S9xReportButton(SDLK_DOWN, false );

		if(event.key.keysym.sym == SDLK_UP)
		S9xReportButton(SDLK_UP, false );

		if(event.key.keysym.sym == SDLK_SPACE)
		S9xReportButton(SDLK_SPACE, false );

		if(event.key.keysym.sym == SDLK_d)
		S9xReportButton(SDLK_d, false );
		
		if(event.key.keysym.sym == SDLK_c)
		S9xReportButton(SDLK_c, false );

		if(event.key.keysym.sym == SDLK_s)
		S9xReportButton(SDLK_s, false );

		if(event.key.keysym.sym == SDLK_x)
		S9xReportButton(SDLK_x, false );

		if(event.key.keysym.sym == SDLK_a)
		S9xReportButton(SDLK_a, false );

		if(event.key.keysym.sym == SDLK_z)
		S9xReportButton(SDLK_z, false );
		
			break;



		case SDL_QUIT:
			quit_state = TRUE;
			break;
		}
	}
	
	if (quit_state == TRUE)
	{
		printf ("Quit Event. Bye.\n");
		S9xExit();
	}
}

bool S9xPollButton (uint32 id, bool *pressed)
{
	return (false);
}

bool S9xPollAxis (uint32 id, int16 *value)
{
	return (false);
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
	return (false);
}

// domaemon: needed by SNES9X
void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}



