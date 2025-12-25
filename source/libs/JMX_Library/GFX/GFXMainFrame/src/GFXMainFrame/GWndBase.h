#pragma once

#include "BSLib/BSLib.h"
#include <Test/Test.h>
#include <Windows.h>
#include <list>
#include <memory/util.h>
/// Holds data about events, mainly in the 3d-space. It's unknown if this structure is also used for the general
/// messaging system.
struct Event3D {
    char pad_0000[4]; //0x0000
    int Msg;          //0x0004
    char pad_0008[12];//0x0008
    int lParam;       //0x0014
    int wParam;       //0x0018
};                    //Size: 0x001C

/**
 * This class acts as a parent for any control and provides a container for child-controls
 * \remark As of current state of knowledge, \ref CGWndBase::OnUpdate must be overwritten in any derived class to
 *         avoid an endless loop
 *
 */
class CGWndBase : public CObjChild {                  // size 32 + 100 = 132
    GFX_DECLARE_DYNAMIC_EXISTING(CGWndBase, 0x0C5DE1C)//ECSRO ORIG

public:
    struct wnd_size {
        int width;
        int height;
    };

    struct wnd_pos {
        int x;
        int y;
    };

    struct wnd_rect {

        int bottom() const {
            return pos.y + size.height;
        }

        int left() const {
            return pos.x;
        }

        int right() const {
            return pos.x + size.width;
        }

        int top() const {
            return pos.y;
        }

        wnd_pos pos;
        wnd_size size;
    };


public:
    /// \address 00B90A80
    virtual void Func_7(CGWndBase *a2);

    /// Add the given control as a child.
    /// \address 00B90AC0
    //virtual int AddControlToList(CGWndBase *a2);

    /// Remove the given control from the list of children.
    /// \address 00B913D0
    virtual void RemoveControlFromList(CGWndBase *a2);

    /// Called when this instance is created.
    /// \address 00B8F1F0
    virtual bool OnCreate(long ln);

    /// Called before this instance is deleted.
    /// \address 00B8F200
    virtual bool OnRelease();

    /// Called once per frame (blocking).
    /// \address 00B92070
    virtual void OnUpdate();

    virtual void RenderMyself();

    virtual void RenderMyChildren();

    virtual void Func_15();

    virtual void Render();

    virtual bool On3DEvent_MAYBE(Event3D *a2);

    virtual bool Func_18(Event3D *a1);

    virtual void OnWndMessage(Event3D *a1);

    virtual void BringToFront();

    /// Resize this control
    /// \param width Width in pixels
    /// \param height Height in pixels
    virtual void SetGWndSize(int width, int height);

    virtual void Func_22(int x, int y);

    /// Set the visibility state of this control.
    /// \param bVisible New visibility state.
    virtual void ShowGWnd(bool bVisible);


public:
    /// Constructor, what shall I say about this?
    /// \address 009ce220
    CGWndBase();

    /// Get visible state of this control
    /// \retval true Control is visible
    /// \retval false Control is not visible
    bool IsVisible();

    void ApplyGlobalScale(int x);

    /// \address 009d1e60
    void sub_9d1e60(bool b);

    /// Returns the numeric identifier of this control.
    /// \returns ID of this control
    int UniqueID() const;

    void SetFocus_MAYBE();

    /// \brief Get the bounds of this object (position and size)
    /// \return The bounds of this object
    wnd_rect GetBounds() const;

    CGWndBase::wnd_pos CGWndBase::GetPos() {
        return this->bounds.pos;
    }

    CGWndBase::wnd_size CGWndBase::GetSize() {
        return this->bounds.size;
    }

    /// \address 00b9db00
    bool IsDragable();

    /// \address 00b9d8f0
    bool SendMessageToParentDispatcher(DWORD dwMSG, DWORD lParam, DWORD wParam);

    /// \address 00b90d90
    void SetPosition(int nX, int nY);

    /// \address 00b9d940
    bool IsParentOf(CGWndBase *pGWnd) {
        for (; pGWnd != NULL; pGWnd = pGWnd->GetParentControl())
            if (this == pGWnd) {
                return true;
            }
        return false;
    }

    /// \address 009cda20
    int GethgWnd();

    // \address 0x009cdab0
    bool IsClickable();

    /// \brief Allow Window to click or move
    /// \address 00b8f540
    void SetClickable(bool bState);

    /// \brief Set the drag window by mouse
    /// \address 00b9daf0
    void SetDragable(bool bState);

protected:
    /// \address 00B8F440
    void sub_B8F440(const RECT &rect);

public:// 0046c8b6 getting call by global ptr
    CGWndBase *GetParentControl();

    std::n_list<CGWndBase *> GetControlList() const;

	template<typename T>
	T* GetControlFromList(int nId) {
		std::n_list<CGWndBase*>::const_iterator it = this->N00000707.begin();
		for (; it != this->N00000707.end(); ++it) {
			if ((*it)->UniqueID() == nId) {
				return (T*)(*it);
			}
		}
		return NULL;
	}

	CGWndBase* GetControlFromList(int nId) {
		return GetControlFromList<CGWndBase>(nId);
	}

	void RemoveControlFromList(int nId) {
		RemoveControlFromList(GetControlFromList(nId));
	}

public:
	int m_lnListLockRead;      //0x0018
	int m_lnListLockWrite;     //0x001C
	std::n_list<CGWndBase*> N00000707;//0x0020
	CGWndBase* m_parentControl;//0x002C
	int m_UniqueID;            //0x0030
	int N000006F9;             //0x0034
	bool N000006FB;            //0x0038
	char pad_0039[3];          //0x0039
	wnd_rect bounds;           //0x003C
	RECT N000007E7;            //0x004C
	char pad_005C[1];          //0x005C
	bool m_bVisible;           //0x005D
	bool m_bClickable;         //0x005E
	bool N0000074F;            //0x005F
	bool N000007F1;            //0x0060
	char pad_0061[3];          //0x0061
	int N00000703;             //0x0064
	int m_iflist;              //0x0068
	short N00000705; //0x0070
	char pad_0072[6]; //0x0072
public:
	std::n_list<CGWndBase*> N00000709; //0x0078
    BEGIN_FIXTURE()
    ENSURE_SIZE(0x006C)
    END_FIXTURE()

    RUN_FIXTURE(CGWndBase)
};//Size: 0x006C
