/*
 *  Copyright (C) 2003-2004  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GUMP_H_INCLUDED
#define GUMP_H_INCLUDED

#include "Object.h"
#include "Rect.h"
#include <list>

class RenderSurface;
class Shape;
class Item;
class GumpNotifyProcess;

//
// Class Gump
//
// Desc: Base Gump Class that all other Gumps inherit from
//

class Gump : public Object
{
protected:

	friend class GumpList;

	uint16				owner;			// Owner item
	Gump *				parent;			// Parent gump
	sint32				x, y;			// Gumps position. This is always the upper left corner!

	Pentagram::Rect		dims;			// The dimentions/coord space of the gump
	uint32				flags;			// Gump flags
	sint32				layer;			// gump ordering layer

	sint32				index;			// 'Index'

	Shape				*shape;			// The gumps shape (always painted at 0,0)
	uint32				framenum;

	//! The Gump list for this gump. This will contain all child gumps,
	//! as well as all gump widgets. 
	std::list<Gump*>	children;		// List of all gumps
	Gump *				focus_child;	// The child that has focus

	uint16				notifier;		// Process to notify when we're closing
	uint32				process_result;	// Result for the notifier process

public:
	ENABLE_RUNTIME_CLASSTYPE();
	Gump();
	Gump(int x, int y, int width, int height, uint16 owner = 0,
		 uint32 _Flags = 0, sint32 layer = LAYER_NORMAL);
	virtual ~Gump();

public:

	virtual void				CreateNotifier();
	GumpNotifyProcess*			GetNotifyProcess();
	inline uint32				GetResult() { return process_result; }

	//! Set the Gump's shape/frame
	inline void					SetShape(Shape *_shape, uint32 _framenum)
		{ shape = _shape; framenum = _framenum; }

	//! Set the Gump's frame
	inline void					SetFramenum(uint32 _framenum)
		{ framenum = _framenum; }

	//! Init the gump, call after construction
	virtual void				InitGump();

	//! Find a gump of the specified type (this or child)
	//! \param t Type of gump to look for
	//! \param recursive Recursively search through children?
	//! \param no_inheritance Exactly this type, or is a subclass also allowed?
	//! \return the desired Gump, or NULL if not found
	virtual Gump *				FindGump(const RunTimeClassType& t,
										 bool recursive=true,
										 bool no_inheritance=false);

	//! Find a gump of the specified type (this or child)
	//! \param T Type of gump to look for
	//! \param recursive Recursively search through children?
	//! \param no_inheritance Exactly this type, or is a subclass also allowed?
	//! \return the desired Gump, or NULL if not found
	template<class T> Gump *	FindGump(bool recursive=true,
										 bool no_inheritance=false)
		{ return FindGump(T::ClassType, recursive, no_inheritance); }

	//! Find gump (this, child or NULL) at parent coordinates (mx,my)
	//! \return the Gump at these coordinates, or NULL if none
	virtual Gump *		FindGump(int mx, int my);

	//! Get the mouse cursor for position mx, my relative to parents position.
	//! If this gump doesn't want to set the cursor, the gump list will
	//! attempt to get the cursor shape from the next lower gump.
	//! \return true if this gump wants to set the cursor, false otherwise
	virtual bool		GetMouseCursor(int mx, int my, Shape &shape,
									   sint32 &frame);

	// Update the RenderSurface of this gump and all children (probably
	//  only needed for scaled gumps).
	//virtual bool		DeviceChanged();

	//! Run the gump
	//! \return true if repaint required
	virtual bool		Run(const uint32 framenum);

	//! Called when there is a map change (so the gumps can self terminate
	//! among other things)
	virtual void		MapChanged(void);

	//! Paint the Gump (RenderSurface is relative to parent).
	//! Calls PaintThis and PaintChildren
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	virtual void		Paint(RenderSurface* surf, sint32 lerp_factor);

protected:

	//! Overloadable method to Paint just this Gump
	//! (RenderSurface is relative to this)
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	virtual void		PaintThis(RenderSurface* surf, sint32 lerp_factor);

	//! Paint the Gumps Children (RenderSurface is relative to this)
	// \param surf The RenderSurface to paint to
	// \param lerp_factor The lerp_factor to paint at (0-256)
	virtual void		PaintChildren(RenderSurface* surf, sint32 lerp_factor);

public:

	//! Close the gump
	//! \param no_del If true, do not delete after closing
	virtual void		Close(bool no_del = false);

	//! Check to see if a Gump is Closing
	bool				IsClosing() { return (flags&FLAG_CLOSING)!=0; }

	//! Move this gump
	virtual void		Move(int x_, int y_)
		{ x = x_ - moveOffsetX; y = y_ - moveOffsetY; }

	enum Position {
		CENTER = 1,
		TOP_LEFT = 2,
		TOP_RIGHT = 3,
		BOTTOM_LEFT = 4,
		BOTTOM_RIGHT = 5
	};

	//! Moves this gump to a relative location on the parent gump
	// \param pos the postition on the parent gump
	// \param xoffset an offset from the position on the x-axis
	// \param yoffset an offset from the position on the y-axis
	virtual void		setRelativePosition(Position pos, int xoffset=0, int yoffset=0);

	//
	// Points and Coords
	//

	//! Get the dims
	virtual void		GetDims(Pentagram::Rect &d) { d = dims; }

	//! Detect if a point is on the gump
	virtual bool		PointOnGump(int mx, int my);

	//! Convert a screen space point to a gump point
	virtual void		ScreenSpaceToGump(int &sx, int &sy);

	//! Convert a gump point to a screen space point
	virtual void		GumpToScreenSpace(int &gx, int &gy);

	//! Convert a parent relative point to a gump point
	virtual void		ParentToGump(int &px, int &py);

	//! Convert a gump point to parent relative point
	virtual void		GumpToParent(int &gx, int &gy);


	//! Trace a click, and return ObjId
	virtual uint16		TraceObjId(int mx, int my);

	//! Get the location of an item in the gump (coords relative to this).
	//! \return false on failure
	virtual bool		GetLocationOfItem(uint16 itemid, int &gx, int &gy,
										  sint32 lerp_factor = 256);


	//
	// Some event handlers. In theory they 'should' be able to be mapped to
	// Usecode classes.
	//
	// mx and my are relative to parents position
	//
	// OnMouseDown returns the Gump that handled the Input, if it was handled.
	// The MouseUp,MouseDouble events will be sent to the same gump.
	//
	// Unhandled input will be passed down to the next lower gump.
	//
	// A mouse click on a gump will make it focus, IF it wants it.
	//
	
	// Return Gump that handled event
	virtual Gump *		OnMouseDown(int button, int mx, int my);
	virtual void		OnMouseUp(int  button, int mx, int my) { }
	virtual void		OnMouseClick(int button, int mx, int my) { }
	virtual void		OnMouseDouble(int button, int mx, int my) { }

	// Keyboard input gets sent to the FocusGump. Or if there isn't one, it
	// will instead get sent to the default key handler. TextInput requires
	// that text mode be enabled. Return true if handled, false if not.
	// Default, returns false, unless handled by focus child
	virtual bool		OnKeyDown(int key, int mod);
	virtual bool		OnKeyUp(int key);
	virtual bool		OnTextInput(int unicode);

	// This is for detecting focus changes for keyboard input. Gets called true
	// when the this gump is being set as the focus focus gump. It is called
	// false when focus is being taken away.
	virtual void		OnFocus(bool /*gain*/) { }
	
	// Makes this gump the focus
	virtual void		MakeFocus();

	// Is this gump the focus?
	inline bool			IsFocus()
		{ return parent?parent->focus_child==this:false; }

	// Get the child in focus
	inline Gump *		GetFocusChild() { return focus_child; }

	// Find a new Child to be the focus
	void				FindNewFocusChild();	


	//
	// Child gump related
	//

	//! Add a gump to the child list. 
	virtual void		AddChild(Gump *, bool take_focus = true);

	//! Remove a gump from the child list
	virtual void		RemoveChild(Gump *);	

	//! Get the parent
	inline Gump *		GetParent() { return parent; }

	//! Get the root gump (or self)
	Gump *				GetRootGump();

	//! This function is used by our children to notifty us of 'something'
	//! Think of it as a generic call back function
	virtual void		ChildNotify(Gump *child, uint32 message) { }
	void				SetIndex(sint32 i) { index = i; }
	sint32				GetIndex() { return index; }

	// Dragging
	//! Called when a child gump starts to be dragged.
	//! \return false if the child isn't allowed to be dragged.
	virtual bool		StartDraggingChild(Gump* gump, int mx, int my);
	virtual void		DraggingChild(Gump* gump, int mx, int my);
	virtual void		StopDraggingChild(Gump* gump);

	//! This will be called when an item in this gump starts to be dragged.
	//! \return false if the item isn't allowed to be dragged.
	virtual bool		StartDraggingItem(Item* item, int mx, int my)
		{ return false; }

	//! Called when an item is being dragged over the gump.
	//! Note: this may be called on a different gump than StartDraggingItem.
	//! \return false if the item can't be dragged to this location.
	virtual bool		DraggingItem(Item* item, int mx, int my)
		{ return false; }

	//! Called when an item that was being dragged over the gump left the gump
	virtual void		DraggingItemLeftGump(Item* item) { }

	//! Called when a drag operation finished.
	//! This is called on the same gump that received StartDraggingItem
	//! \param moved If true, the item was actually dragged somewhere else.
	//!              If false, the drag was cancelled.
	virtual void		StopDraggingItem(Item* item, bool moved) { }

	//! Called when an item has been dropped on a gump.
	//! This is called after StopDraggingItem has been called, but possibly
	//! on a different gump.
	//! It's guaranteed that a gump will only receive a DropItem at a location
	//! if a DraggingItem there returned true.
	virtual void		DropItem(Item* item, int mx, int my) { }

	//! the MoveOffset is the point relative to which Move() will move the gump
	void				SetMoveOffset(int mx, int my)
		{ moveOffsetX = mx; moveOffsetY = my; }
protected:
	int moveOffsetX, moveOffsetY;

public:

	//
	// Gump Flags
	//
	enum GumpFlags {
		FLAG_UNMOVABLE		= 0x01,		// When set, the gump can not be dragged
		FLAG_HIDDEN			= 0x02,		// When set, the gump will not be drawn
		FLAG_CLOSING		= 0x04,		// When set, the gump is closing
		FLAG_CLOSE_AND_DEL	= 0x08,		// When set, the gump is closing and will be deleted
		FLAG_ITEM_DEPENDANT	= 0x10		// When set, the gump will be deleted on MapChange
	};

	inline bool			IsHidden() { return (flags&FLAG_HIDDEN) != 0; }
	virtual void		HideGump() { flags |= FLAG_HIDDEN; }
	virtual void		UnhideGump() { flags &= ~FLAG_HIDDEN; }

	//
	// Gump Layers
	//
	enum GumpLayers {
		LAYER_DESKTOP		= -16,		// Layer for Desktop 'bottom most'
		LAYER_GAMEMAP		= -8,		// Layer for the World Gump
		LAYER_NORMAL		= 0,		// Layer for Normal gumps
		LAYER_ABOVE_NORMAL	= 8,		// Layer for Always on top Gumps
		LAYER_MODAL         = 12,		// Layer for Modal Gumps
		LAYER_CONSOLE		= 16		// Layer for the console
	};

	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif //GUMP_H_INCLUDED
