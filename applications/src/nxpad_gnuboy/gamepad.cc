/* Copyright © 2008 - Fabien GIGANTE */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <map>
#include <set>
#define MWINCLUDECOLORS
#include "microwin/nano-X.h"

using namespace std;

//---------------------------------------------------------------------------------------------------------------------------------
class CKeyboard
//---------------------------------------------------------------------------------------------------------------------------------
{
  set<int> prKeys;

  void SendEvent(int key, bool pressed)
  {
    GrInjectKeyboardEvent(GrGetFocus(), key, 0/*modifiers*/, 0/*scancode*/, pressed);
  }

public:

  set<int>* GetPressedKeys() { return &prKeys; }

  bool IsKeyPressed(int key) { return prKeys.find(key)!=prKeys.end(); }

  bool IsOneOfKeysPressed(const set<int>& keys)
  {
    for (set<int>::iterator ikey=keys.begin(); ikey!=keys.end(); ikey++) 
      if (IsKeyPressed(*ikey)) return true;
    return false; 
  }

  bool CompareKeysPressed(const set<int>& keys)
  {
    if (keys.size()!=prKeys.size()) return false;
    for (set<int>::iterator ikey=keys.begin(); ikey!=keys.end(); ikey++) 
      if (!IsKeyPressed(*ikey)) return false;
    return true;
  }

  void PressKey(int key) { SendEvent(key, true); prKeys.insert(key); }

  void ReleaseKey(int key) { SendEvent(key,false); prKeys.erase(key); }

  void PressKeys(const set<int>& keys)
  {
    for (set<int>::iterator ikey=keys.begin(); ikey!=keys.end(); ikey++) 
      PressKey(*ikey);
  }

  void ReleaseKeys(const set<int>& keys)
  {
    for (set<int>::iterator ikey=keys.begin(); ikey!=keys.end(); ikey++) 
      ReleaseKey(*ikey);
  }

  void ReleaseAllKeys()
  {
    for (set<int>::iterator ikey=prKeys.begin(); ikey!=prKeys.end(); ikey++)
      SendEvent(*ikey,false);
    prKeys.clear();
  }

};

//---------------------------------------------------------------------------------------------------------------------------------
class CSkinMap
//---------------------------------------------------------------------------------------------------------------------------------
{
  unsigned long bgColor;
  char* bitmapFile[2];
  map<int,GR_RECT> keyMap;
  map<int,GR_RECT> cmdMap;
  map<int,int> propMap;
  GR_RECT lockRect;

public:

  CSkinMap() 
  {
    bgColor = WHITE; 
    bitmapFile[0] = bitmapFile[1] = NULL; 
  }

  virtual ~CSkinMap()
  {
    if(bitmapFile[0]) free(bitmapFile[0]), bitmapFile[0]=NULL; 
    if(bitmapFile[1]) free(bitmapFile[1]), bitmapFile[1]=NULL; 
  }

  unsigned long GetBgColor() { return bgColor; }

  GR_RECT GetRectKey(int key) { return keyMap[key]; }

  GR_RECT GetRectCmd(int cmd) { return cmdMap[cmd]; }

  const char* GetBitmapFile(bool pressed) { return bitmapFile[pressed]; }

  void Parse(const char* file)
  {
    FILE* fd = fopen(file,"rt");
    if (!fd) { fprintf(stderr, "nxpad: cannot open skin file %s\n",file); exit(1); }
    while (!feof(fd))
    {
      // read a line
      char line[256]; 
      if (!fgets(line,256,fd)) break;
      while (line[strlen(line)-1]==10 || line[strlen(line)-1]==13)
        line[strlen(line)-1]=0;
      // ignore blank lines and comments
      if (line[0]==0 || line[0]=='#') continue;
      char* words[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
      // split into words
      char* word = strtok(line," ");
      int nargs=0; 
      while (word && nargs<6)
        if (strlen(word))
        {
          words[nargs]=word;
          word=strtok(NULL, " ");
          nargs++; 
        }
      // interpret command
      if (strcasecmp(words[0],"bg")==0)
      {
        if (nargs<4) continue;
        bgColor = GR_RGB( atoi(words[1]), atoi(words[2]), atoi(words[3]) );
      }
      else if (strcasecmp(words[0],"set")==0)
      {
        if (nargs<3) continue;
        int key = words[1][0];
        propMap[key] = atoi(words[2]);
      }
      else if (strcasecmp(words[0],"map")==0)
      {
        if (nargs<6) continue;
        int key = (words[1][0]=='\'' || words[1][0]=='\"') ? words[1][1] : atoi(words[1]);
        GR_RECT r = { atoi(words[2]), atoi(words[3]), atoi(words[4]), atoi(words[5]) };
        keyMap[key] = r;
      }
      else if (strcasecmp(words[0],"cmd")==0)
      {
        if (nargs<6) continue;
        int key = words[1][0];
        GR_RECT r = { atoi(words[2]), atoi(words[3]), atoi(words[4]), atoi(words[5]) };
        cmdMap[key] = r;
      }
      else if (strcasecmp(words[0],"bitmap")==0)
      {
        if (nargs<3) continue;
        int key = atoi(words[1]);
        if (key<0 || key>1 || bitmapFile[key]) continue;
        bitmapFile[key]=strdup(words[2]);
      }
    }
    fclose(fd);
  }

  bool IsInside(int x, int y, GR_RECT &r )
  {
    return (x>=r.x && y>=r.y && x<r.x+r.width && y<r.y+r.height);
  }

  void MapToKeys(int x, int y, set<int>& keys)
  {
    for (map<int,GR_RECT>::iterator pPos=keyMap.begin(); pPos!=keyMap.end(); pPos++) 
      if (IsInside(x,y,pPos->second)) keys.insert(pPos->first);
  }

  int MapToCmd(int x, int y)
  {
    for (map<int,GR_RECT>::iterator pPos=cmdMap.begin(); pPos!=cmdMap.end(); pPos++) 
      if (IsInside(x,y,pPos->second)) return pPos->first;
    return 0;
  }

  int GetProperty(int key)
  {
    map<int,int>::iterator pPos=propMap.find(key);
    if (pPos!=propMap.end()) return pPos->second;
    return -1;
  }

};

//---------------------------------------------------------------------------------------------------------------------------------
class CSkin
//---------------------------------------------------------------------------------------------------------------------------------
{
  CSkinMap skinMap; 
  GR_IMAGE_ID   bitmap[2];
  GR_IMAGE_INFO info;

  void LoadBitmap(int i, const char* file)
  {
    if (i<0 || i>1 || !file || bitmap[i]) return;
    char path[PATH_MAX+1];
    getcwd(path,PATH_MAX);
    strcat(path,"/");
    strcat(path,file);
    bitmap[i] = GrLoadImageFromFile(path, 0);
  }

public:

  CSkin() { bitmap[0] = bitmap[1] = 0; }

  int GetWidth() { return info.width; }
  int GetHeight() { return info.height; }
  unsigned long GetBgColor() { return skinMap.GetBgColor(); }
  bool HasCaption() { return skinMap.GetProperty('c')!=0; }

  void Load(const char* file)
  {
    skinMap.Parse(file ? file : "nxpad-skin.txt");
    LoadBitmap(0, skinMap.GetBitmapFile(false)); 
    LoadBitmap(1, skinMap.GetBitmapFile(true));
    if (!bitmap[0]) fprintf(stderr,"nxpad: can't load skin bitmap\n");
    GrGetImageInfo(bitmap[0], &info);
  }

  void ApplyMask(GR_WINDOW_ID win)
  {
    /* TODO */
    //GR_REGION_ID rid1 = GrNewBitmapRegion(eyemask_bits, info.width, info.height);
      //GrSetWindowRegion(win, rid1, GR_WINDOW_BOUNDING_MASK);
      //GrDestroyRegion(rid1);
  }

  void MapToKeys(int x, int y, set<int>& keys) { skinMap.MapToKeys(x,y,keys); }
  int MapToCmd(int x, int y) { return skinMap.MapToCmd(x,y); }

  void Free()
  {
    if (bitmap[0]) GrFreeImage(bitmap[0]),bitmap[0]=0;
    if (bitmap[1]) GrFreeImage(bitmap[1]),bitmap[1]=0;
  }

  void Paint(GR_WINDOW_ID win, GR_GC_ID gc, set<int>* keys, set<int>* cmds)
  { 
    GrDrawImageToFit(win, gc, 0, 0, -1, -1, bitmap[0]); 
    if (!keys && !cmds) return;
    if (bitmap[1])
    {
      GR_REGION_ID region =    GrNewRegion();
      if (keys) for (set<int>::iterator ikey=keys->begin(); ikey!=keys->end(); ikey++) 
      {
        GR_RECT rect = skinMap.GetRectKey(*ikey);
        GrUnionRectWithRegion(region, &rect);
      }
      if (cmds) for (set<int>::iterator icmd=cmds->begin(); icmd!=cmds->end(); icmd++) 
      {
        GR_RECT rect = skinMap.GetRectCmd(*icmd);
        GrUnionRectWithRegion(region, &rect);
      }
      GrSetGCRegion(gc, region);
      GrDrawImageToFit(win, gc, 0, 0, -1, -1, bitmap[1]); 
      GrSetGCRegion(gc, 0);
      GrDestroyRegion(region);
    }
    else
    {
      GrSetGCMode(gc, GR_MODE_XOR);
      if (keys) for (set<int>::iterator ikey=keys->begin(); ikey!=keys->end(); ikey++) 
      {
        GR_RECT rect = skinMap.GetRectKey(*ikey);
        GrFillRect(win, gc, rect.x, rect.y, rect.width, rect.height);
      }
      if (cmds) for (set<int>::iterator icmd=cmds->begin(); icmd!=cmds->end(); icmd++) 
      {
        GR_RECT rect = skinMap.GetRectCmd(*icmd);
        GrFillRect(win, gc, rect.x, rect.y, rect.width, rect.height);
      }
      GrSetGCMode(gc, GR_MODE_SET);
    }
  }

};

//---------------------------------------------------------------------------------------------------------------------------------
class CPad
//---------------------------------------------------------------------------------------------------------------------------------
{
  CKeyboard    kbd;
  CSkin        skin;
  bool         lock;
  bool         quit;
  bool         repaint;
  bool         dragging, moving;
  int          moveX, moveY;
  GR_WINDOW_ID win;             
  GR_GC_ID     gc;

public:

  CPad()
  {
    win=0; gc=0;
    lock=false;
    quit=false; repaint=false; 
    dragging=false; moving=false; moveX=moveY=0;
  } 

  void Create(const char* skinFile)
  {
    if (GrOpen() < 0) { fprintf(stderr, "nxpad: cannot open graphics\n"); exit(1); }
    skin.Load(skinFile);
    GR_SCREEN_INFO si; GrGetScreenInfo(&si);
    int x = (si.cols-skin.GetWidth())/2, y = 4*si.rows/5 - skin.GetHeight();
    win = GrNewWindow(GR_ROOT_WINDOW_ID, x, y, skin.GetWidth(), skin.GetHeight(), 0/*bordersize*/, skin.GetBgColor(), 0/*border color*/);
    skin.ApplyMask(win);
    GR_WM_PROPERTIES props;
    props.flags = GR_WM_FLAGS_PROPS;
    props.props = GR_WM_PROPS_NOFOCUS | GR_WM_PROPS_NORAISE ; /*GR_WM_PROPS_NOBACKGROUND*/
    if (skin.HasCaption())
    {
      props.flags |= GR_WM_FLAGS_TITLE;
      props.props |= GR_WM_PROPS_CAPTION | GR_WM_PROPS_CLOSEBOX;
      props.title = (char*) "Game Pad";
    } 
    else props.props |= GR_WM_PROPS_NODECORATE;
    GrSetWMProperties(win, &props);
    gc = GrNewGC();
  }

  void Close()
  {
    GrDestroyGC(gc);
    skin.Free();
    GrClose();
  }

  void Paint()
  {
    set<int> cmds; if (lock) cmds.insert('l');
    skin.Paint(win,gc,kbd.GetPressedKeys(), &cmds);
  }

  void OnMouseUp(int x, int y)
  {
    dragging = moving = false;
    if (!moving && !lock) { kbd.ReleaseAllKeys(); repaint=true; }
  }

  void OnMouseDown(int x, int y)
  {
    dragging = true;
    int cmd = skin.MapToCmd(x, y);
    switch (cmd)
    {
    case 0: // must be a key
      {
        set<int> keys;
        skin.MapToKeys(x, y, keys);
        if (!keys.empty())
        {
          if (lock && kbd.IsOneOfKeysPressed(keys)) kbd.ReleaseKeys(keys);
          else kbd.PressKeys( keys );
          // CLM repaint = true;
        }
        else if (!skin.HasCaption())
        {
          moving=true; moveX=x; moveY=y;
        }
      }
      break;
    case 'l': // lock
      lock=!lock;
      if (!lock) kbd.ReleaseAllKeys();
      repaint=true;
      break;
    case 'q':
      quit=true;
      break;
    }
  }

  void OnMouseMove(int rx, int ry, int x, int y)
  {
    if (moving) GrMoveWindow(win, rx-moveX, ry-moveY);
    else if (dragging && !lock)
    {
      set<int> keys;
      skin.MapToKeys(x, y, keys);
      if (!kbd.CompareKeysPressed(keys))
      {
        kbd.ReleaseAllKeys();
        kbd.PressKeys( keys );
        repaint = true;
      }
    }
  }

  void Run()
  {
    GR_EVENT_MASK mask = GR_EVENT_MASK_CLOSE_REQ | GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_UP | GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_MOUSE_POSITION;
    GrSelectEvents(win, mask);
    GrMapWindow(win);
    while (!quit)
    {
      GR_EVENT event;
      GrGetNextEvent(&event);
      switch(event.type)
      {
      case GR_EVENT_TYPE_CLOSE_REQ:
        quit=true;
        break;
      case GR_EVENT_TYPE_EXPOSURE:
        repaint=true;
        break;
      case GR_EVENT_TYPE_BUTTON_UP:
        OnMouseUp(event.button.x, event.button.y);
        break;
      case GR_EVENT_TYPE_BUTTON_DOWN:
        OnMouseDown(event.button.x, event.button.y);
        break;
      case GR_EVENT_TYPE_MOUSE_POSITION:
        OnMouseMove(event.mouse.rootx, event.mouse.rooty, event.mouse.x, event.mouse.y);
        break;
      }
      if (repaint) { Paint(); repaint=false; }
    }
    Close();
  }
};

//---------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
//---------------------------------------------------------------------------------------------------------------------------------
{
  CPad pad;
  pad.Create( argc>=2 ? argv[1] : NULL );
  pad.Run();
};
