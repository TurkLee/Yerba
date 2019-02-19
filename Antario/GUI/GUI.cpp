#include "GUI.h"
#include "..\Settings\Settings.h"
#include <execution>
#include <algorithm>
#include "..\Utils\Images.h"

#pragma warning(disable:4996)

using namespace ui;

/* Defined to avoid including windowsx.h */
#define GET_X_LPARAM(lp)                        (int(short(LOWORD(lp))))
#define GET_Y_LPARAM(lp)                        (int(short(HIWORD(lp))))

/* No inline vars for shared pointers I guess */
MenuStyle MenuMain::style;                          /* Struct containing all colors / paddings in our menu */
Control*  MenuMain::pFocusedObject;                 /* Pointer to the focused object */
std::shared_ptr<CD3DFont>    MenuMain::pFont;       /* Pointer to the font used in the menu */
std::unique_ptr<MouseCursor> MenuMain::mouseCursor; /* Pointer to our mouse cursor */


namespace KeyState
{
	bool keys[256];
	bool oldKeys[256];

	bool GetKeyPress(unsigned int key)
	{
		if (keys[key] == true && oldKeys[key] == false)
			return true;
		else
			return false;
	}
}

void MouseCursor::Render()
{
	const auto x = this->vecPointPos.x;
	const auto y = this->vecPointPos.y;

	/* Draw inner fill color */
	SPoint ptPos1 = { x + 1,  y + 1 };
	SPoint ptPos2 = { x + 25, y + 12 };
	SPoint ptPos3 = { x + 12, y + 25 };
	//g_Render.TriangleFilled(ptPos1, ptPos2, ptPos3, MenuMain::style.colCursor);

	/* Draw second smaller inner fill color */
	ptPos1 = { x + 6,  y + 6 };
	ptPos2 = { x + 19, y + 12 };
	ptPos3 = { x + 12, y + 19 };
	//g_Render.TriangleFilled(ptPos1, ptPos2, ptPos3, MenuMain::style.colCursor);

	/* Draw border */
	//g_Render.Triangle({ x, y }, { x + 25, y + 12 }, { x + 12, y + 25 }, Color::Black(200));
}


void MouseCursor::RunThink(const UINT uMsg, const LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_NCMOUSEMOVE:
		this->SetPosition(SPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		break;
	case WM_LBUTTONDOWN:
		this->bLMBPressed = true;
		break;
	case WM_LBUTTONUP:
		this->bLMBHeld = false;
		this->bLMBPressed = false;
		break;
	case WM_RBUTTONDOWN:
		this->bRMBPressed = true;
		break;
	case WM_RBUTTONUP:
		this->bRMBHeld = false;
		this->bRMBPressed = false;
		break;
	default:
		break;
	}
	if (this->bLMBPressed)
	{
		if (this->bLMBHeld)
			this->bLMBPressed = false;

		this->bLMBHeld = true;
	}
	if (this->bRMBPressed)
	{
		if (this->bRMBHeld)
			this->bRMBPressed = false;
		this->bRMBHeld = true;
	}
}

bool MouseCursor::IsInBounds(const SPoint& vecDst1, const SPoint& vecDst2)
{
	return this->IsInBounds({ vecDst1, vecDst2 });
}


bool MouseCursor::IsInBounds(const SRect& rcRect)
{
	return rcRect.ContainsPoint(this->GetPos());
}


void UIObject::SetupBaseSize()
{
	this->rcBoundingBox.right = rcBoundingBox.left + szSizeObject.x;
	this->rcBoundingBox.bottom = rcBoundingBox.top + szSizeObject.y;
}

void Control::RequestFocus()
{
	if (pFocusedObject == this)
		return;

	if (!this->CanHaveFocus())
		return;

	if (pFocusedObject)
		pFocusedObject->OnFocusOut();

	pFocusedObject = dynamic_cast<Control*>(this);
	pFocusedObject->OnFocusIn();
}

void MenuMain::Render()
{
	/*  Render all children */
	for (auto& it : this->vecChildren)
		if (it.get() != pFocusedObject)
			it->Render();

	/* Render focused object as the last one */
	if (pFocusedObject)
		pFocusedObject->Render();
}

bool MenuMain::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	mouseCursor->RunThink(uMsg, lParam);
	this->UpdateData();

	/* Loop through all of the child objects and capture the input */
	if (!this->vecChildren.empty() && g_Settings.bMenuOpened)
	{
		for (auto& it : this->vecChildren)
			if (it->MsgProc(uMsg, wParam, lParam))
				return true;
	}
	return false;
}

bool MenuMain::UpdateData()
{
	if (!this->vecChildren.empty() && g_Settings.bMenuOpened)
	{
		for (auto& it : this->vecChildren)
			if (it->UpdateData())
				return true;        /* Return true if our updatedata did change something. */
	}
	return false;
}

void Section::AddDummy()
{
	this->AddChild(std::make_shared<DummySpace>(SPoint(this->GetMaxChildWidth(), pFont->iHeight + 6)));
}

void Section::AddCheckBox(const std::string& strSelectableLabel, bool* bValue)
{
	this->AddChild(std::make_shared<Checkbox>(strSelectableLabel, bValue, GetThis()));
}

void Section::AddButton(const std::string& strSelectableLabel, void(&fnPointer)(), SPoint vecButtonSize)
{
	this->AddChild(std::make_shared<Button>(strSelectableLabel, fnPointer, GetThis(), vecButtonSize));
}

void Section::AddCombo(const std::string& strSelectableLabel, int* iVecIndex, std::vector<std::string> vecBoxOptions)
{
	this->AddChild(std::make_shared<ComboBox>(strSelectableLabel, vecBoxOptions, iVecIndex, GetThis()));
}

void Section::AddList(const std::string& strSelectableLabel, int iSize, int* iVecIndex, std::vector<std::string> vecBoxOptions)
{
	this->AddChild(std::make_shared<ListBox>(strSelectableLabel, vecBoxOptions, iSize, iVecIndex, GetThis()));
}

void Section::AddMulti(const std::string& strSelectableLabel, bool* bEnabled, std::vector<std::string> vecBoxOptions)
{
	this->AddChild(std::make_shared<MultiBox>(strSelectableLabel, vecBoxOptions, bEnabled, GetThis()));
}

void Section::AddSlider(const std::string& strLabel, std::string strAddon, float* flValue, float flMinValue, float flMaxValue)
{
	this->AddChild(std::make_shared<Slider<float>>(strLabel, strAddon, flValue, flMinValue, flMaxValue, GetThis()));
}

void Section::AddSlider(const std::string& strLabel, std::string strAddon, int* iValue, int iMinValue, int iMaxValue)
{
	this->AddChild(std::make_shared<Slider<int>>(strLabel, strAddon, iValue, iMinValue, iMaxValue, GetThis()));
}

void Section::AddText(const std::string& strLabel)
{
	this->AddChild(std::make_shared<Text>(strLabel, GetThis()));
}

void Section::AddKeyBind(const std::string& strSelectableLabel, ButtonCode_t * key, int * iCurrentValue, SPoint vecButtonSize)
{
	this->AddChild(std::make_shared<KeyPicker>(strSelectableLabel, key, iCurrentValue, GetThis(), vecButtonSize));
}

void Section::AddColor(const std::string& strLabelName, Color* colorOutput)
{
	this->AddChild(std::make_shared<ColorPicker>(strLabelName, colorOutput, GetThis()));
}

void Section::AddTextField(const std::string& strSelectableLabel, std::string * strInput, SPoint vecButtonSize)
{
	this->AddChild(std::make_shared<TextField>(strSelectableLabel, strInput, GetThis(), vecButtonSize));
}

ObjectPtr ControlManager::GetObjectAtPoint(SPoint ptPoint)
{
	ObjectPtr returnObject = nullptr;
	std::for_each(std::execution::par, vecChildren.begin(), vecChildren.end(),
		[&ptPoint, &returnObject](ObjectPtr& object)
	{
		const auto control = std::dynamic_pointer_cast<Control>(object);
		if (object->GetBBox().ContainsPoint(ptPoint) && (!control || control->GetUsable()))
		{
			object->SetHovered(true);
			returnObject = object;
		}
		else object->SetHovered(false);
	});
	return returnObject;
}


void ControlManager::RenderChildObjects()
{
	for (auto& it : this->vecChildren)
		if (it.get() != pFocusedObject)
			it->Render();
}


void ControlManager::SetupPositions()
{
	this->SetupBaseSize();
	this->SetupChildPositions();
}


void ControlManager::SetupChildPositions()
{
	std::for_each(std::execution::par, vecChildren.begin(), vecChildren.end(),
		[](ObjectPtr& object)
	{
		object->SetupPositions();
	});
}


Window::Window(const std::string& strLabel, SPoint szSize, std::shared_ptr<CD3DFont> pMainFont, std::shared_ptr<CD3DFont> pFontHeader)
{
	this->pFont = pMainFont;
	this->pHeaderFont = pFontHeader;
	this->strLabel = strLabel;
	this->szSizeObject = szSize;
	this->bIsDragged = false;

	this->iHeaderWidth = 0;
	UIObject::SetPos(g_Render.GetScreenCenter() - (szSize * .5f));
	this->type = TYPE_WINDOW;
}

void Window::Render()
{
	g_Render.Rect(this->rcBoundingBox.Pos() - 7, { this->rcBoundingBox.right + 7, this->rcBoundingBox.top - 26 }, Color(160, 160, 160, g_Settings.iAlpha));

	g_Render.Rect(this->rcBoundingBox.Pos() - 7, { this->rcBoundingBox.right + 7, this->rcBoundingBox.bottom + 6 }, Color(160, 160, 160, g_Settings.iAlpha));

	g_Render.RectFilled(this->rcBoundingBox.Pos() - 6, { this->rcBoundingBox.right + 7, this->rcBoundingBox.top - 25 }, Color(35, 35, 35, g_Settings.iAlpha));

	g_Render.RectFilled(this->rcBoundingBox.Pos() - 6, { this->rcBoundingBox.right + 7, this->rcBoundingBox.bottom + 6 }, Color(30, 30, 30, g_Settings.iAlpha));

	//D3DLOCKED_RECT	   tex_locked;
	//LPDIRECT3DTEXTURE9 background = NULL;

	//D3DXCreateTextureFromFileInMemoryEx(g_Render.pDevice, lfaith, sizeof(lfaith), 550, 420, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &background);

	//if (!background)
	//	return;

	//if (background->LockRect(0, &tex_locked, nullptr, 0) != D3D_OK)
	//	return;

	//g_Render.createSprite(background, lfaith, sizeof(lfaith), 550, 420);
	//g_Render.Sprite(background, this->rcBoundingBox.left, this->rcBoundingBox.top, 0.0f);

	g_Render.RectFilledGradient(this->rcBoundingBox.Pos() - 6, { this->rcBoundingBox.right + 7, this->rcBoundingBox.top + 2 },
		Color(25, 25, 25, g_Settings.iAlpha), Color(30, 30, 30, g_Settings.iAlpha), GradientType::GRADIENT_VERTICAL);

	/* Draw background header */
	//g_Render.RectFilled(this->rcBoundingBox.Pos() - 6, { this->rcBoundingBox.right + 6, this->rcBoundingBox.top - 19 }, Color(35, 35, 35, g_Settings.iAlpha));

	/* Draw main background rectangle */
	//g_Render.RectFilled(this->rcBoundingBox, Color(30, 30, 30, g_Settings.iAlpha));

	/* Draw main background outline */
	//g_Render.Rect(this->rcBoundingBox.Pos() - 1, { this->rcBoundingBox.right + 1, this->rcBoundingBox.bottom + 1 }, Color(1, 1, 1, g_Settings.iAlpha));

	/* Draw header string, defined as label. */
	g_Render.String(this->rcBoundingBox.left, this->rcBoundingBox.top - 22, CD3DFONT_DROPSHADOW,
		Color(255, 255, 255, g_Settings.iAlpha), this->pFont.get(), this->strLabel.c_str());

	SIZE sz;
	std::string watermark = __DATE__ " | Ensidiya";

	g_Fonts.pFontTahoma8->GetTextExtent(watermark.c_str(), &sz);

	std::transform(watermark.begin(), watermark.end(), watermark.begin(), ::tolower);

	//g_Render.String(this->rcBoundingBox.right - 10 - sz.cx, this->rcBoundingBox.bottom - 11, CD3DFONT_DROPSHADOW,
		//Color(255, 255, 255, g_Settings.iAlpha), this->pFont.get(), watermark);

	/* Render all children */
	this->RenderChildObjects();
}


void Window::Initialize()
{
	this->tSelectedTab = std::dynamic_pointer_cast<Tab>(*vecChildren.begin());  /* Setup the first selected tab as the first obj. */
	this->tSelectedTab->SetActive(true);
	this->SetupPositions();
	for (auto& it : vecChildren)
		it->Initialize();
}


void Window::SetupPositions()
{
	/* Set base window rect */
	this->SetupBaseSize();
	this->rcHeader =
	{
		this->rcBoundingBox.left + 7,
		this->rcBoundingBox.top - 25,
		this->rcBoundingBox.right + 7,
		this->rcBoundingBox.top
	};

	/* Make tabs take up all of aviable window width */
	const int iSingleTabHeight = rcBoundingBox.Height() / vecChildren.size();

	/* Setup tab positions */
	int iUsedSpace = 0;
	for (auto& it : vecChildren)
	{
		it->SetSize({ 103, iSingleTabHeight });
		it->SetPos({ this->GetPos().x + iHeaderWidth, this->GetPos().y + iUsedSpace });
		iUsedSpace += iSingleTabHeight;
		it->SetupPositions();
	}
}

bool Window::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (this->ptOldMousePos == SPoint(0, 0))
		this->ptOldMousePos = mouseCursor->GetPos();

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		/* Handle window dragging */
		if (this->bIsDragged)
		{
			int screenWidth, screenHeight;
			g_pEngine->GetScreenSize(screenWidth, screenHeight);
			int w = screenWidth;
			int h = screenHeight;

			this->SetPos(this->GetPos() + mouseCursor->GetPos() - this->ptOldMousePos);

			this->SetPos(SPoint(std::clamp(static_cast<int>(this->GetPos().x), 6, static_cast<int>(w)
				- static_cast<int>(this->GetSize().x + 7)), std::clamp(static_cast<int>(this->GetPos().y), 24, static_cast<int>(h)
					- static_cast<int>(this->GetSize().y + 6))));
			this->SetupPositions();
			return true;
		}
	case WM_LBUTTONDOWN:
	{
		/* Check what tab mouse is hovering */
		auto tHoveredTab = std::dynamic_pointer_cast<Tab>(this->GetObjectAtPoint(mouseCursor->GetPos()));
		if (tHoveredTab)
		{
			/* Handle tab selection */
			if (mouseCursor->bLMBPressed)
			{
				tSelectedTab->SetActive(false);
				tHoveredTab->SetActive(true);
				tSelectedTab = tHoveredTab;
				/* Clear focus on tabchange */
				if (pFocusedObject)
				{
					pFocusedObject->OnFocusOut();
					pFocusedObject = nullptr;
				}
				return true;
			}
		}
		/* Check if the window is dragged. If it is, move window by the cursor difference between ticks. */
		if (this->rcHeader.ContainsPoint(mouseCursor->GetPos()) && uMsg == WM_LBUTTONDOWN)
		{
			this->bIsDragged = true;
			return true;
		}
		break;
	}
	case WM_LBUTTONUP:
		this->bIsDragged = false;
		break;
	}

	this->ptOldMousePos = mouseCursor->GetPos();


	std::copy(KeyState::keys, KeyState::keys + 255, KeyState::oldKeys);
	for (int x = 0; x < 255; x++)
	{
		KeyState::keys[x] = (GetAsyncKeyState(x));
	}

	/* Run MsgProc for selected tab if we did't capture input yet */
	return this->tSelectedTab->MsgProc(uMsg, wParam, lParam);
}

std::shared_ptr<Section> Tab::AddSection(const std::string& strLabel, float flPercSize)
{
	auto tmp = std::make_shared<Section>(strLabel, flPercSize, GetThis());
	this->AddChild(tmp);
	tmp->SetParent(GetThis());
	return tmp;
}

ScrollBar::ScrollBar(ObjectPtr pParentObject)
{
	this->pParent = pParentObject;
	this->iPageSize = 0;
	this->szSizeObject.x = 8;
	this->flScrollAmmount = 0;
	this->bIsVisible = true; /* For initials checks */
	this->eState = CLEAR;
	this->bIsVisible = true;/* For initials checks */
}


void ScrollBar::Initialize()
{
	this->szSizeObject.y = pParent->GetSize().y - 2;
	this->SetupPositions();
}


void ScrollBar::Render()
{
	if (!this->bIsVisible)
		return;

	/* Background */
	g_Render.RectFilled(this->rcBox, Color(30, 30, 30, 255));

	/* Up/down button */
	g_Render.RectFilled(this->rcUpButton, Color(40, 40, 40, 255));
	g_Render.RectFilled(this->rcDownButton, Color(40, 40, 40, 255));

	/* Drag thumb */
	g_Render.RectFilled(this->rcDragThumb, g_Settings.Miscellaneous.cMenuColor);

	/* Has to be here as wndproc is not called when you hold lmb and not do anything else */
	this->HandleArrowHeldMode();
}


bool ScrollBar::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	{
		if (mouseCursor->IsInBounds(rcDragThumb))
			eHoveredButton = THUMB;
		else if (mouseCursor->IsInBounds(rcUpButton))
			eHoveredButton = UP;
		else if (mouseCursor->IsInBounds(rcDownButton))
			eHoveredButton = DOWN;
		else if (mouseCursor->IsInBounds(rcBoundingBox))
			eHoveredButton = SHAFT;
		else
			eHoveredButton = NONE;

		bIsHovered = eHoveredButton != NONE;
	}
	case WM_LBUTTONDOWN:
	{
		if (!mouseCursor->bLMBHeld)
			bIsThumbUsed = false;

		if (bIsHovered && uMsg == WM_LBUTTONDOWN)
		{
			/* Handle button behaviour */
			switch (eHoveredButton)
			{
			case THUMB:
			{
				if (mouseCursor->bLMBPressed)
				{
					this->RequestFocus();
					bIsThumbUsed = true;
				}
				break;
			}
			case UP:
			{
				if (mouseCursor->bLMBPressed)
				{
					this->RequestFocus();

					this->eState = CLICKED_UP;
					this->flScrollAmmount -= 1;
					UpdateThumbRect();
					this->pParent->SetupPositions();
					return true;
				}
				break;
			}
			case DOWN:
			{
				if (mouseCursor->bLMBPressed)
				{
					this->RequestFocus();

					this->eState = CLICKED_DOWN;
					this->flScrollAmmount -= 1;
					UpdateThumbRect();
					this->pParent->SetupPositions();
					return true;
				}
				break;
			}
			case SHAFT:
			{
				if (mouseCursor->bLMBPressed)
				{
					this->RequestFocus();
					this->eState = CLEAR;
					this->flScrollAmmount += mouseCursor->GetPos().y > rcDragThumb.top ? iPageSize : -iPageSize;
					UpdateThumbRect();
					this->pParent->SetupPositions();
					return true;
				}
			}
			case NONE:
				this->eState = CLEAR;
				break;
			}
		}
		if (bIsThumbUsed)
		{
			if (ptOldMousePos == SPoint(0, 0))
				ptOldMousePos = mouseCursor->GetPos();

			/* Scale the mouse movement accordingly */
			auto diff = mouseCursor->GetPos().y - ptOldMousePos.y;
			const auto scale = [](int in, int bmin, int bmax, int lmin, int lmax) {
				return (float((lmax - lmin) * (in - bmin)) / float((bmax - bmin))) + lmin;
			};

			/* Change the scroll ammount by difference in pixels of the old and new mousepos scaled accordingly */
			flScrollAmmount += scale(diff, 0, rcDownButton.top - rcUpButton.bottom - 4 - rcDragThumb.Height(), 0, pParent->GetScrollableHeight());
			UpdateThumbRect();
			this->pParent->SetupPositions();

			return true;
		}
	}
	break;
	case WM_MOUSEWHEEL:
	{
		if (mouseCursor->IsInBounds(pParent->GetBBox()))
		{
			this->RequestFocus();
			this->flScrollAmmount -= 10 * int(float(GET_WHEEL_DELTA_WPARAM(wParam)) / float(WHEEL_DELTA));
			UpdateThumbRect();
			this->pParent->SetupPositions();
			return true;
		}
	}
	}

	ptOldMousePos = mouseCursor->GetPos();
	return false;
}


void ScrollBar::SetupPositions()
{
	if (!this->bIsVisible)
		return;

	this->iPageSize = pParent->GetSize().y - style.iPaddingY * 2;
	this->rcBoundingBox.left = pParent->GetBBox().right - szSizeObject.x + 5;
	this->rcBoundingBox.right = rcBoundingBox.left + szSizeObject.x - 6;
	this->rcBoundingBox.top = pParent->GetBBox().top + 1;
	this->rcBoundingBox.bottom = rcBoundingBox.top + szSizeObject.y;

	this->rcUpButton = { rcBoundingBox.left, rcBoundingBox.top, rcBoundingBox.right, rcBoundingBox.top + szSizeObject.x };
	this->rcDownButton = { rcBoundingBox.left, rcBoundingBox.bottom - szSizeObject.x, rcBoundingBox.right, rcBoundingBox.bottom };
	this->rcDragThumb.left = rcUpButton.left;
	this->rcDragThumb.right = rcUpButton.right;

	/* Thumb size, -4 cus of space between top and the bottom button */
	this->sizeThumb = { szSizeObject.x, max(int(float((rcDownButton.top - rcUpButton.bottom) * iPageSize) / float(pParent->GetScrollableHeight() + iPageSize - 4)), style.iMinThumbSize) };

	UpdateThumbRect();
}


void ScrollBar::UpdateThumbRect()
{
	const auto iScrollableHeight = pParent->GetScrollableHeight();

	if (iScrollableHeight <= 0)
	{
		/* Nothing to scroll through */
		rcDragThumb.top = rcUpButton.bottom + 2;
		rcDragThumb.bottom = rcDownButton.top - 2;
		flScrollAmmount = 0;
	}
	else
	{
		/*Make sure we won't exceed out of bounds */
		this->flScrollAmmount = std::clamp(flScrollAmmount, 0.f, float(iScrollableHeight));
		/* 2 offset from buttons(so +2), and the position is scaled with the scrollable height (size of the aviable area for thumb / scrHeight) */
		rcDragThumb.top = rcUpButton.bottom + 2 + (flScrollAmmount * (rcBoundingBox.Height() - (szSizeObject.x + 2) * 2 - sizeThumb.y) / iScrollableHeight);
		rcDragThumb.bottom = rcDragThumb.top + sizeThumb.y;
	}
}


void ScrollBar::HandleArrowHeldMode()
{
	if (mouseCursor->bLMBHeld)
	{
		switch (eHoveredButton)
		{
		case UP:
		{
			this->eState = HELD_UP;
			this->flScrollAmmount -= 1;
			UpdateThumbRect();
			this->pParent->SetupPositions();
			break;
		}
		case DOWN:
		{
			this->eState = HELD_DOWN;
			this->flScrollAmmount += 1;
			UpdateThumbRect();
			this->pParent->SetupPositions();
			break;
		}
		}
	}
}


Tab::Tab(const std::string& strTabName, int iNumColumns, ObjectPtr parentWindow)
{
	this->bIsHovered = false;
	this->bIsActive = false;
	this->iColumnCount = iNumColumns;
	this->strLabel = strTabName;
	this->pParent = parentWindow;
	this->type = TYPE_TAB;
}


void Tab::Initialize()
{
	/* Remove padding from aviable section space */
	const auto iAvWidthForSection = pParent->GetBBox().Width() - style.iPaddingX - 2 * style.iPaddingX - 100;
	this->iMaxChildWidth = iAvWidthForSection / iColumnCount;

	std::for_each(std::execution::par, vecChildren.begin(), vecChildren.end(),
		[](ObjectPtr it) {it->Initialize(); });

	SetupPositions();
}


void Tab::Render()
{
	/* Tab inside, for now menustyle color */
	g_Render.RectFilled(this->rcBoundingBox.Pos(), { this->rcBoundingBox.right + 1, this->rcBoundingBox.bottom },
		bIsActive ? g_Settings.Miscellaneous.cMenuColor(g_Settings.iAlpha) : Color(60, 60, 60, g_Settings.iAlpha));

	g_Render.RectFilled(this->rcBoundingBox, Color(30, 30, 30, g_Settings.iAlpha));

	/* Render tab name */
	g_Render.String(rcBoundingBox.Mid(), CD3DFONT_CENTERED_X | CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, bIsActive ? Color(230, 230, 230, g_Settings.iAlpha) : Color(110, 110, 110, g_Settings.iAlpha),
		g_Fonts.pFontTabs.get(), strLabel.c_str());

	if (bIsHovered)
		g_Render.String(rcBoundingBox.Mid(), CD3DFONT_CENTERED_X | CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, bIsActive ? Color(240, 240, 240, g_Settings.iAlpha) : Color(140, 140, 140, g_Settings.iAlpha),
			g_Fonts.pFontTabs.get(), strLabel.c_str());

	//if (this->bIsHovered)
		//g_Render.RectFilled(rcBoundingBox, style.colHover);

	/* Render sections */
	if (this->bIsActive)
		this->RenderChildObjects();
}


bool Tab::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (this->bIsActive)
	{
		for (auto& it : this->vecChildren)
			if (it->MsgProc(uMsg, wParam, lParam))
				return true;
	}
	return false;
}

bool Tab::UpdateData()
{
	if (this->bIsActive)
	{
		for (auto& it : this->vecChildren)
			if (it->UpdateData())
				return true;
	}
	return false;
}


void Tab::SetupChildPositions()
{
	int iUsedArea = 0,
		iPosX = this->GetBBox().left + style.iPaddingX;
	int iPosY = pParent->GetBBox().top + style.iPaddingY - 5;

	for (auto& it : this->vecChildren)
	{
		[this, &iUsedArea, &iPosX, &it](int posy)
		{
			posy += iUsedArea;

			/* If section exceeds the bounds move it to the right */
			if (posy + it->GetSize().y > pParent->GetBBox().bottom)
			{
				posy -= iUsedArea;
				iUsedArea = 0;
				iPosX += GetMaxChildWidth() + style.iPaddingX;
			}

			it->SetPos({ iPosX + 105, posy });
			iUsedArea += it->GetSize().y + style.iPaddingY;

			it.get()->SetupPositions();
		}(iPosY);
	}
}


Section::Section(const std::string& strLabel, float flPercSize, ObjectPtr parentTab)
{
	this->pParent = parentTab;
	this->strLabel = strLabel;
	this->type = TYPE_SECTION;

	flPercSize = std::clamp(flPercSize, 0.f, 3.f);
	this->flSizeScale = flPercSize;
}

void Section::Render()
{
	g_Render.RectFilled(this->rcBoundingBox, Color(30, 30, 30, g_Settings.iAlpha));

	g_Render.SetCustomScissorRect(this->rcBoundingBox);
	{
		scrollBar->Render();
		this->RenderChildObjects();
	}
	g_Render.RestoreOriginalScissorRect();

	g_Render.RectFilled(this->rcBoundingBox.Pos(), { this->rcBoundingBox.right - 5, this->rcBoundingBox.top + 8 }, Color(30, 30, 30, g_Settings.iAlpha));

	g_Render.Rect(this->rcBoundingBox.Pos() + 1, { this->rcBoundingBox.right - 1, this->rcBoundingBox.bottom - 1 }, Color(25, 25, 25, g_Settings.iAlpha));
	g_Render.Rect(this->rcBoundingBox, Color(1, 1, 1, g_Settings.iAlpha));

	g_Render.String(this->rcBoundingBox.Section(), CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, Color(255, 255, 255, g_Settings.iAlpha), pFont.get(), strLabel.c_str());
}


void Section::RenderChildObjects()
{
	for (auto& it : this->vecChildren)
	{
		auto control = std::dynamic_pointer_cast<Control>(it);
		if (control.get() != pFocusedObject && control->GetVisible())
			control->Render();
	}
}


void Section::Initialize()
{
	this->szSizeObject =
	{
		pParent->GetMaxChildWidth(),
		int(float(pParent->GetParent()->GetBBox().Height() - pParent->GetBBox().Height()) * flSizeScale) - style.iPaddingY
	};

	scrollBar->Initialize();
	this->iMaxChildWidth = this->szSizeObject.x - 2 * style.iPaddingX;
	for (auto& it : vecChildren)
		it->Initialize();

	SetupPositions();

	/* If there are not enough controls for scrollbar to be useful - disable it */
	if (this->GetScrollableHeight() <= 0)
		this->scrollBar->SetVisible(false);
}


bool Section::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* Let the scrollbar handle input first */
	if (this->scrollBar->HandleMouseInput(uMsg, wParam, lParam))
		return true;

	/* Then let the focused control handle full msgproc if defined (colorpickers / any other popup windows) */
	if (pFocusedObject && GetThis() == pFocusedObject->GetParent())
		if (pFocusedObject->MsgProc(uMsg, wParam, lParam))
			return true;

	/* And after that all the other controls */
	switch (uMsg)
	{

		/* Keyboard input */
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	case WM_CHAR:
	{
		/* Let the focused control handle the input, no focus = no keyboard handling */
		if (pFocusedObject)
			if (pFocusedObject->HandleKeyboardInput(uMsg, wParam, lParam))
				return true;
		break;
	}

	/* Mouse input */
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
	{
		/* Let the focused object handle mouse input before the rest */
		if (pFocusedObject)
			if (pFocusedObject->HandleMouseInput(uMsg, wParam, lParam))
				return true;

		bool bHandledInput = false;
		auto pControl = this->GetObjectAtPoint(mouseCursor->GetPos());

		/* If hovered control is not the focused one, we didnt capture input for it yet so do it */
		if (pControl.get() != pFocusedObject)
		{
			const auto oldFocus = pFocusedObject;

			if (pControl)
				if (pControl->HandleMouseInput(uMsg, wParam, lParam))
					bHandledInput = true;

			/* Our control changed focused object */
			if (oldFocus != pFocusedObject)
				return bHandledInput;

			/* If the new object is not a focused one OR you press LMB out-of-bouds of focused obj, LOSE FOCUS */
			if (uMsg == WM_LBUTTONDOWN && pFocusedObject)
			{
				if ((bHandledInput && pFocusedObject != pControl.get()) || (!bHandledInput && GetThis() == pFocusedObject->GetParent()))
				{
					pFocusedObject->OnFocusOut();
					pFocusedObject = nullptr;
				}
			}
		}
		return bHandledInput;
	}
	}

	return false;
}


void Section::SetupChildPositions()
{
	/* Create scrollbar object, this function is called first at init - thats why here */
	if (!scrollBar)
		this->scrollBar = std::make_unique<ScrollBar>(GetThis());

	/* Setup positions of out scrollbar */
	this->scrollBar->SetupPositions();

	/* Saved used area of section. Used for control alignment. */
	int iUsedArea = 0 - this->scrollBar->GetScrollAmmount();
	for (auto& it : vecChildren)
	{
		/* Useful cast for later use */
		auto control = std::dynamic_pointer_cast<Control>(it);

		const int iPosX = this->rcBoundingBox.left + 40,
			iPosY = this->rcBoundingBox.top + style.iPaddingY + iUsedArea;


		/* Check if we will exceed bounds of the section and do not render the selectable */
		if (iPosY > this->GetBBox().bottom || iPosY + control->GetSize().y < this->GetPos().y)
		{
			control->SetVisible(false);
			control->SetUsable(false);
		}
		else
		{
			/* If any part of the control is out-of-bounds, render it but dont capture input */
			control->SetVisible(true);

			if (iPosY + it->GetSize().y > this->GetBBox().bottom || iPosY < this->GetPos().y)
				control->SetUsable(false);
			else
				control->SetUsable(true);
		}

		it->SetPos({ iPosX, iPosY });
		it->SetupPositions();

		iUsedArea += it->GetSize().y + 6;
	}
	/* Save full section height - all controls etc. */
	this->iTotalPixelHeight = iUsedArea + this->scrollBar->GetScrollAmmount() + style.iPaddingY;
}

Text::Text(const std::string& strLabel, ObjectPtr pParent)
{
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->szSizeObject.x = 10;
	this->szSizeObject.y = 10;
	this->type = TYPE_TEXT;
}


void Text::Render()
{
	g_Render.String(this->rcBox.right + int(float(style.iPaddingX) - 6), this->rcBoundingBox.Mid().y,
		CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, style.colText(g_Settings.iAlpha), pFont.get(), this->strLabel.c_str());
}


void Text::SetupPositions()
{
	this->SetupBaseSize();

	/* ------ this will be moved to other function not called all the time ------- */
	this->rcBoundingBox.right = this->rcBoundingBox.left + this->szSizeObject.x;
	this->rcBoundingBox.bottom = this->rcBoundingBox.top + this->szSizeObject.y;

	this->rcBox = [this]()
	{
		auto diff = (this->rcBoundingBox.Height()) / 2;
		return SRect(rcBoundingBox.left + diff - 10, rcBoundingBox.top + diff, rcBoundingBox.left + diff - 10, rcBoundingBox.bottom - diff);
	}();
}

Checkbox::Checkbox(const std::string& strLabel, bool *bValue, ObjectPtr pParent)
{
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->bCheckboxValue = bValue;
	this->szSizeObject = { 100, int(float(pFont->iHeight - 3) * 1.4) };
	this->type = TYPE_CHECKBOX;
}


void Checkbox::Render()
{
	/* Checkbox background */
	g_Render.RectFilledGradient(this->rcBox, Color(70, 70, 70, g_Settings.iAlpha), style.colComboBoxRect, GradientType::GRADIENT_VERTICAL);

	/* Fill the inside of the button depending on activation */
	if (*this->bCheckboxValue)
		g_Render.RectFilledGradient(this->rcBox, g_Settings.Miscellaneous.cMenuColor(g_Settings.iAlpha), g_Settings.Miscellaneous.cMenuColor(130), GradientType::GRADIENT_VERTICAL);

	/* If the button is hovered, make it lighter */
	if (this->bIsHovered || mouseCursor->IsInBounds(this->rcBox))
		g_Render.RectFilled(this->rcBox, style.colHover);

	/* Render the outline */
	g_Render.Rect(this->rcBox, Color(1, 1, 1, g_Settings.iAlpha));

	/* Render button label as its name */
	g_Render.String(this->rcBox.right + 16, this->rcBoundingBox.Mid().y,
		CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, style.colText(g_Settings.iAlpha), pFont.get(), this->strLabel.c_str());
}


bool Checkbox::UpdateData()
{
	if (!this->bIsVisible)
		return false;

	if (mouseCursor->IsInBounds(this->rcBox) && mouseCursor->bLMBPressed)
	{
		*this->bCheckboxValue = !*this->bCheckboxValue;
		this->bIsActive = false;
		return true;
	}
}


bool Checkbox::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (mouseCursor->bLMBPressed && (mouseCursor->IsInBounds(rcBoundingBox) || mouseCursor->bLMBPressed))
		{
			*this->bCheckboxValue = !*this->bCheckboxValue;
			this->bIsActive = false;
			return true;
		}
	}

	return false;
}


void Checkbox::SetupPositions()
{
	this->SetupBaseSize();

	/* ------ this will be moved to other function not called all the time ------- */
	this->rcBoundingBox.right = this->rcBoundingBox.left + this->szSizeObject.x;
	this->rcBoundingBox.bottom = this->rcBoundingBox.top + this->szSizeObject.y;

	this->rcBox = [this]()
	{
		auto size = int(float(rcBoundingBox.Height()) * 0.8f);
		auto diff = (this->rcBoundingBox.Height() - size) / 2;
		return SRect(rcBoundingBox.left + diff - 23, rcBoundingBox.top + diff, rcBoundingBox.left + diff + size - 24, rcBoundingBox.bottom - diff - 2);
	}();
}


KeyPicker::KeyPicker(const std::string& strLabel, ButtonCode_t * key, int * iCurrentValue, ObjectPtr pParent, SPoint ptKeyPickerSize)
{
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->key = key;
	this->bIsActivated = false;
	this->type = TYPE_KEYPICKER;
	this->bPressed = false;

	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;
	this->szSelectableSize = { this->szSizeObject.x, pFont->iHeight + int(float(style.iPaddingY) * 0.5f) };
	this->keyName = "";
	this->iCurrentValue = iCurrentValue;
}


void KeyPicker::Render()
{
	/* Fill the body of the KeyPicker */
	g_Render.RectFilled(this->rcBoundingBox, Color(39, 39, 39, g_Settings.iAlpha));

	if (*key == BUTTON_CODE_NONE)
		keyName = "";
	else
		keyName = g_pInputSystem->ButtonCodeToString(*key);

	std::transform(keyName.begin(), keyName.end(), keyName.begin(), ::tolower);

	/* Text inside the KeyPicker */
	if (!bPressed)
		g_Render.String(this->rcBoundingBox.Button(), CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, *this->iCurrentValue == 0 ? style.colTextOut(g_Settings.iAlpha) : Color(160, 160, 160, g_Settings.iAlpha),
			pFont.get(), *this->iCurrentValue == 0 ? "always on" : keyName);

	if (this->bIsHovered)
		g_Render.RectFilled(this->rcBoundingBox, style.colHover);

	if (bPressed)
		g_Render.String(this->rcBoundingBox.Button(), CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, g_Settings.Miscellaneous.cMenuColor(g_Settings.iAlpha), pFont.get(), "press key");

	/* Rectangle of the keypicker selectables, for scissorrect */
	const SRect vpTypes =
	{
		rcBoundingBox.left + 10,
		rcBoundingBox.bottom,
		rcBoundingBox.right,
		rcBoundingBox.bottom + rcBoundingBox.Height() * 4
	};

	if (bIsActive)
	{
		g_Render.RectFilled(this->rcBoundingBox.right - 69, this->rcBoundingBox.bottom + 3,
			this->rcBoundingBox.right, this->rcBoundingBox.bottom + rcBoundingBox.Height() * 4, Color(35, 35, 35, g_Settings.iAlpha));

		if (this->idHovered != -1)
		{
			const auto rcElementBounds = [this]() -> SRect
			{
				int posy = this->rcBoundingBox.bottom + 1 + this->rcBoundingBox.Height() * this->idHovered;
				return {
					this->rcBoundingBox.right - 69,
					posy + 1,
					this->rcBoundingBox.right - 1,
					posy + this->rcBoundingBox.Height() - 1
				};
			};

			g_Render.RectFilled(rcElementBounds(), Color(25, 25, 25, g_Settings.iAlpha));
		}

		auto index = 0;
		const auto ptMid = this->rcBoundingBox.Button();
		for (const auto& it : vecSelectables) {
			g_Render.String({ rcBoundingBox.right - 55, ptMid.y + 1 + this->rcBoundingBox.Height() * (index++ + 1) },
				CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, Color(158, 158, 158, g_Settings.iAlpha), pFont.get(), it.c_str());
		}

		if (*this->iCurrentValue != -1)
		{
			g_Render.String({ this->rcBoundingBox.right - 55, this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * *this->iCurrentValue + 10 },
				CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, g_Settings.Miscellaneous.cMenuColor(g_Settings.iAlpha), pFont.get(), this->vecSelectables[*this->iCurrentValue].c_str());
		}

		g_Render.Rect(this->rcBoundingBox.right - 69, this->rcBoundingBox.bottom + 3,
			this->rcBoundingBox.right, this->rcBoundingBox.bottom + rcBoundingBox.Height() * 4, style.colSectionOutl(g_Settings.iAlpha));
	}
	/* KeyPicker outline */
	g_Render.Rect(this->rcBoundingBox, style.colSectionOutl(g_Settings.iAlpha));
}


void KeyPicker::Initialize()
{
	/* Basically override if the size was not specified, its run only once anyway */
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;
	this->ptSelectablePosition =
	{
		this->rcBoundingBox.left,
		this->rcBoundingBox.top + int(float(style.iPaddingY) - 2)
	};

	vecSelectables = std::vector<std::string>
	{
		"always on",
		"on hotkey",
		"toggle",
		"off hotkey"
	};
}


bool KeyPicker::UpdateData()
{
	if (!this->bIsVisible)
		return false;

	if (bPressed)
	{
		for (int i = 0; i < 255; i++)
		{
			if (KeyState::GetKeyPress(i))
			{
				if (i == VK_LBUTTON || i == VK_RBUTTON)
				{
					return false;
				}

				if (i == VK_MBUTTON)
				{
					*key = MOUSE_MIDDLE; bPressed = false; return false;
				}

				if (i == VK_XBUTTON1)
				{
					*key = MOUSE_4; bPressed = false; return false;
				}

				if (i == VK_XBUTTON2)
				{
					*key = MOUSE_5; bPressed = false; return false;
				}

				*key = g_pInputSystem->VirtualKeyToButtonCode(i);

				switch (i)
				{
				case 1: *key = MOUSE_LEFT; break;
				case 2: *key = MOUSE_RIGHT; break;
				}

				if (*key == KEY_ESCAPE)
				{
					*key = BUTTON_CODE_NONE;
					bPressed = false;
					return false;
				}

				bPressed = false;
				return false;
			}
		}
	}

	return false;
}


bool KeyPicker::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_RBUTTONDOWN:
		if (this->bIsHovered)
		{
			this->idHovered = -1;
			if (mouseCursor->bRMBPressed && mouseCursor->IsInBounds(rcBoundingBox))
			{
				this->bIsActive = !bIsActive;
				/* If active, request focus so its rendered on top of every other selectable */
				if (this->bIsActive)
					this->RequestFocus();
				return true;
			}
		}
	case WM_LBUTTONDOWN:
		/* If the main selectabled is hovered, handle only its input */
		if (this->bIsActive && mouseCursor->IsInBounds({ this->rcBoundingBox.right - 69, this->rcBoundingBox.bottom,
		this->rcBoundingBox.right, this->rcBoundingBox.bottom + rcBoundingBox.Height() * 4 }))
		{
			for (std::size_t it = 0; it < this->vecSelectables.size(); ++it)
			{
				/* Bounds of the looped element. */
				const auto rcElementBounds = [this, it]()
				{
					int posy = this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * it;
					return SRect(this->rcBoundingBox.left, posy, this->rcBoundingBox.right, posy + this->rcBoundingBox.Height());
				};

				/* If we don't hover the element - ignore */
				if (!mouseCursor->IsInBounds(rcElementBounds()))
					continue;

				this->idHovered = it;
				if (mouseCursor->bLMBPressed)
				{
					*this->iCurrentValue = it;
					this->idHovered = -1;
					this->bIsActive = false;
					return true;
				}
			}
		}
		else
			this->idHovered = -1;
	case WM_LBUTTONDBLCLK:
		if (mouseCursor->bLMBPressed && mouseCursor->IsInBounds(rcBoundingBox))
		{
			this->RequestFocus();
			this->bPressed = !bPressed;
			return true;
		}
	}

	return false;
}


extern char* lowercase[254];
extern char* uppercase[254];


TextField::TextField(const std::string & strLabel, std::string * strInput, ObjectPtr pParent, SPoint ptKeyPickerSize)
{
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->input = strInput;
	this->bIsActivated = false;
	this->type = TYPE_TEXTFIELD;
	this->bPressed = false;
	/* Will be overriden on the init if == 0, 0 */
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;
	this->szSelectableSize = { this->szSizeObject.x, pFont->iHeight + int(float(style.iPaddingY) * 0.5f) };
}


void TextField::Render()
{
	/* Fill the body of the KeyPicker */
	g_Render.RectFilled(this->rcBoundingBox, Color(39, 39, 39, g_Settings.iAlpha));

	/* Text inside the KeyPicker */
	if (!bPressed)
		g_Render.String(this->rcBoundingBox.Button(), CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, Color(160, 160, 160, g_Settings.iAlpha), pFont.get(), *input);

	if (this->bIsHovered)
	{
		g_Render.RectFilled(this->rcBoundingBox, style.colHover);
	}

	std::string blinker;

	if (GetTickCount64() >= blink)
	{
		blink = GetTickCount() + 800;
	}

	if (bPressed && GetTickCount64() > (blink - 400))
	{
		blinker += "_";
	}

	if (bPressed)
		g_Render.String(this->rcBoundingBox.Button(), CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, Color(170, 170, 170, g_Settings.iAlpha), pFont.get(), *input + blinker);

	if (input->length() >= 1)
	{
		g_Render.String(this->rcBoundingBox.right - 8, this->rcBoundingBox.bottom - 9, CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, Color(160, 160, 160, g_Settings.iAlpha), pFont.get(), "x");

		if (mouseCursor->IsInBounds({ this->rcBoundingBox.right - 10, this->rcBoundingBox.top }, { this->rcBoundingBox.right, this->rcBoundingBox.bottom }))
			g_Render.String(this->rcBoundingBox.right - 8, this->rcBoundingBox.bottom - 9, CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, Color(185, 185, 185, g_Settings.iAlpha), pFont.get(), "x");
	}

	if (input->length() == 0 && !bPressed)
		g_Render.String(this->rcBoundingBox.Button(), CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_Y, style.colTextOut(g_Settings.iAlpha), pFont.get(), "...");

	/* KeyPicker outline */
	g_Render.Rect(this->rcBoundingBox, style.colSectionOutl(g_Settings.iAlpha));
}


void TextField::Initialize()
{
	this->SetupBaseSize();

	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;

	this->ptSelectablePosition =
	{
		this->rcBoundingBox.left,
		this->rcBoundingBox.top + int(float(style.iPaddingY) - 2)
	};
}


bool TextField::UpdateData()
{
	if (!this->bIsVisible)
		return false;

	std::string str = *input;

	SIZE sz;

	g_Fonts.pFontTahoma8->GetTextExtent(str.c_str(), &sz);

	static bool stopWriting;

	if (sz.cx > this->szSizeObject.x - 27)
		stopWriting = false;
	else
		stopWriting = true;

	if (mouseCursor->IsInBounds({ this->rcBoundingBox.right - 10, this->rcBoundingBox.top }, { this->rcBoundingBox.right, this->rcBoundingBox.bottom }) &&
		mouseCursor->bLMBPressed)
	{
		*input = str.substr(0, strlen(str.c_str()) - strlen(str.c_str()));
		bPressed = false;
	}

	if (bPressed)
	{
		for (int i = 0; i < 255; i++)
		{
			if (KeyState::GetKeyPress(i))
			{
				if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
				{
					bPressed = false;
					return false;
				}

				if (strlen(str.c_str()) != 0) {
					if (GetAsyncKeyState(VK_BACK)) {
						*input = str.substr(0, strlen(str.c_str()) - 1);
					}
				}

				if (stopWriting && i != NULL && uppercase[i] != nullptr) {
					if (GetAsyncKeyState(VK_SHIFT)) {
						*input = str + uppercase[i];
					}
					else {
						*input = str + lowercase[i];
					}

					return false;
				}

				if (stopWriting && i == 32) {
					*input = str + " ";
					return false;
				}
			}
		}
	}

	return false;
}


bool TextField::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (mouseCursor->IsInBounds({ this->rcBoundingBox.right - 10, this->rcBoundingBox.top }, { this->rcBoundingBox.right, this->rcBoundingBox.bottom }))
			return false;

		if (mouseCursor->bLMBPressed)
		{
			this->bPressed = !bPressed;
			return true;
		}
	}

	return false;
}


char* lowercase[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
"y", "z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "/?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "[", "\\", "]", "'", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };


char* uppercase[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, ")", "!", "@", "#", "$", "%", "^", "&", "*", "(", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "_", ".", "?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "{", "|", "}", "\"", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };


Button::Button(const std::string& strLabel, void(&fnPointer)(), ObjectPtr pParent, SPoint ptButtonSize)
{
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->fnActionPlay = fnPointer;
	this->bIsActivated = false;
	this->type = TYPE_BUTTON;
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;
	this->bPressed = false;
	this->szSelectableSize = { this->szSizeObject.x, pFont->iHeight + int(float(style.iPaddingY) * 0.5f) };
}

void Button::Render()
{
	/* Fill the body of the button */
	g_Render.RectFilled(this->rcBoundingBox, Color(39, 39, 39, g_Settings.iAlpha));

	/* Button outline */
	g_Render.Rect(this->rcBoundingBox, Color(1, 1, 1, g_Settings.iAlpha));

	/* Text inside the button */
	g_Render.String(this->rcBoundingBox.Mid(), CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_X | CD3DFONT_CENTERED_Y, Color(160, 160, 160, g_Settings.iAlpha), pFont.get(), this->strLabel.c_str());

	/* If button is pressed */
	if (this->bPressed) {
		g_Render.String(this->rcBoundingBox.Mid(), CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_X | CD3DFONT_CENTERED_Y, g_Settings.Miscellaneous.cMenuColor(g_Settings.iAlpha), pFont.get(), this->strLabel.c_str());
	}

	if (this->bIsHovered)
		g_Render.RectFilled(this->rcBoundingBox, style.colHover);
}


void Button::Initialize()
{
	/* Basically override if the size was not specified, its run only once anyway */
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;
}


bool Button::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (mouseCursor->bLMBPressed)
		{
			this->bPressed = true;
			this->fnActionPlay(); /* Run the function passed as an arg. */
			return true;
		}
	case WM_LBUTTONUP:
		this->bPressed = false;
		return true;
	}
	return false;
}


ListBox::ListBox(const std::string& strLabel, std::vector<std::string> vecBoxOptions, int iSize, int* iCurrentValue, ObjectPtr pParent)
{
	this->type = TYPE_LIST;
	this->pParent = pParent;
	//this->strLabel		 = strLabel;
	this->idHovered = -1;
	this->iCurrentValue = iCurrentValue;
	this->vecSelectables = vecBoxOptions;
	this->iSize = iSize;
	this->bPressed = false;
	this->strLabel = strLabel;
	this->iCurrentValue = iCurrentValue;
}

void ListBox::Initialize()
{
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = (iSize * 18) + 18;
}

void ListBox::Render()
{
	/* Render the selectable with the value in the middle */
	g_Render.RectFilled(this->rcBoundingBox, Color(39, 39, 39, g_Settings.iAlpha));

	/* Highlight selectable if its selected (-1 = no hovered one) */
	if (this->idHovered != -1)
	{
		const auto rcElementBounds = [this]() -> SRect
		{
			int posy = this->rcBoundingBox.bottom + 1 + 18 * this->idHovered;
			return {
				this->rcBoundingBox.left + 1,
				posy + 1,
				this->rcBoundingBox.right - 1,
				posy + 18 - 1
			};
		};

		g_Render.RectFilled(rcElementBounds(), Color(25, 25, 25, g_Settings.iAlpha));
	}

	/* Render all of the selectable labels (names) in the middle */
	auto index = 0;
	const auto ptMid = this->rcBoundingBox.Button();
	for (const auto& it : vecSelectables) {
		g_Render.String({ ptMid.x, rcBoundingBox.top + 18 * (index++ + 1) + 1 },
			CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, Color(158, 158, 158, g_Settings.iAlpha), pFont.get(), it.c_str());
	}

	if (*this->iCurrentValue != -1)
	{
		g_Render.String({ this->rcBoundingBox.left + 10, this->rcBoundingBox.top + 18 * *this->iCurrentValue + 19 },
			CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, g_Settings.Miscellaneous.cMenuColor, pFont.get(), this->vecSelectables[*this->iCurrentValue].c_str());
	}
	/* Render selectables only within the rect, useful for scroll usage (if implemented later on) */

	g_Render.Rect(this->rcBoundingBox.Pos(), { this->rcBoundingBox.right, this->rcBoundingBox.top + 18 }, style.colSectionOutl(g_Settings.iAlpha));
	g_Render.Rect(this->rcBoundingBox, style.colSectionOutl(g_Settings.iAlpha));
}

bool ListBox::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	{
		/* If the main selectabled is hovered, handle only its input */
		if (this->bIsHovered)
		{
			this->idHovered = -1;
			if (mouseCursor->bLMBPressed)
			{
				this->bIsActive = !bIsActive;
				/* If active, request focus so its rendered on top of every other selectable */
				if (this->bIsActive)
					this->RequestFocus();
				return true;
			}
		}
		else if (mouseCursor->IsInBounds({ this->rcBoundingBox.left, this->rcBoundingBox.bottom, this->rcBoundingBox.right,
									  this->rcBoundingBox.bottom + 18 * int(this->vecSelectables.size()) }))
		{
			for (std::size_t it = 0; it < this->vecSelectables.size(); ++it)
			{
				/* Bounds of the looped element. */
				const auto rcElementBounds = [this, it]()
				{
					int posy = this->rcBoundingBox.bottom + 18 * it;
					return SRect(this->rcBoundingBox.left, posy, this->rcBoundingBox.right, posy + 18);
				};

				/* If we don't hover the element - ignore */
				if (!mouseCursor->IsInBounds(rcElementBounds()))
					continue;

				this->idHovered = it;
				if (mouseCursor->bLMBPressed)
				{
					*this->iCurrentValue = it;
					this->idHovered = -1;
					this->bIsActive = false;
					return true;
				}
			}
		}
		else
			this->idHovered = -1;
	}
	}

	return false;
}

ComboBox::ComboBox(const std::string& strLabel, std::vector<std::string> vecBoxOptions, int* iCurrentValue, ObjectPtr pParent)
{
	this->type = TYPE_COMBO;
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->idHovered = -1;
	this->iCurrentValue = iCurrentValue;
	this->vecSelectables = vecBoxOptions;
}


void ComboBox::Initialize()
{
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;
}


void ComboBox::Render()
{
	/* Render the selectable with the value in the middle */
	g_Render.RectFilled(this->rcBoundingBox, Color(39, 39, 39, g_Settings.iAlpha));
	g_Render.String(this->rcBoundingBox.Button(), CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, Color(160, 160, 160, g_Settings.iAlpha), pFont.get(),
		this->vecSelectables[*this->iCurrentValue].c_str());

	if (this->bIsHovered)
	{
		g_Render.RectFilled(this->rcBoundingBox, style.colHover);
	}

	/* Render the small triangle */
	{
		SPoint ptPosMid, ptPosLeft, ptPosRight;

		/* Draw two different versions (top-down, down-top) depending on activation */
		if (!this->bIsActive)
		{
			ptPosMid.x = this->rcBoundingBox.right - 8;
			ptPosRight.x = this->rcBoundingBox.right - 5;
			ptPosLeft.x = this->rcBoundingBox.right - 10;

			ptPosRight.y = ptPosLeft.y = this->rcBoundingBox.top + 8;
			ptPosMid.y = this->rcBoundingBox.bottom - 7;
		}
		else
		{
			ptPosMid.x = this->rcBoundingBox.right - 8;
			ptPosRight.x = this->rcBoundingBox.right - 5;
			ptPosLeft.x = this->rcBoundingBox.right - 11;

			ptPosRight.y = ptPosLeft.y = this->rcBoundingBox.bottom - 7;
			ptPosMid.y = this->rcBoundingBox.top + 7;
		}

		g_Render.TriangleFilled(ptPosLeft, ptPosRight, ptPosMid, Color(162, 162, 162, g_Settings.iAlpha));
	}
	/*-------------------------*/

	/* Rectangle of the combo selectables, for scissorrect */
	const SRect vpCombo =
	{
		rcBoundingBox.left,
		rcBoundingBox.bottom,
		rcBoundingBox.right,
		rcBoundingBox.bottom + rcBoundingBox.Height() * int(vecSelectables.size())
	};

	/* Render selectables only within the rect, useful for scroll usage (if implemented later on) */
	g_Render.SetCustomScissorRect(vpCombo);
	if (this->bIsActive)
		RenderSelectables();
	g_Render.RestoreOriginalScissorRect();

	g_Render.Rect(this->rcBoundingBox, style.colSectionOutl(g_Settings.iAlpha));

	if (bIsActive) {
		g_Render.Rect({ this->rcBoundingBox.left, this->rcBoundingBox.bottom + 3 },
			{ this->rcBoundingBox.right, this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * int(this->vecSelectables.size()) },
			style.colSectionOutl(g_Settings.iAlpha));
	}
}


bool ComboBox::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	{
		/* If the main selectabled is hovered, handle only its input */
		if (this->bIsHovered)
		{
			this->idHovered = -1;
			if (mouseCursor->bLMBPressed)
			{
				this->bIsActive = !bIsActive;
				/* If active, request focus so its rendered on top of every other selectable */
				if (this->bIsActive)
					this->RequestFocus();
				return true;
			}
		}
		else if (this->bIsActive &&
			mouseCursor->IsInBounds({ this->rcBoundingBox.left, this->rcBoundingBox.bottom, this->rcBoundingBox.right,
									  this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * int(this->vecSelectables.size()) }))
		{
			for (std::size_t it = 0; it < this->vecSelectables.size(); ++it)
			{
				/* Bounds of the looped element. */
				const auto rcElementBounds = [this, it]()
				{
					int posy = this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * it;
					return SRect(this->rcBoundingBox.left, posy, this->rcBoundingBox.right, posy + this->rcBoundingBox.Height());
				};

				/* If we don't hover the element - ignore */
				if (!mouseCursor->IsInBounds(rcElementBounds()))
					continue;

				this->idHovered = it;
				if (mouseCursor->bLMBPressed)
				{
					*this->iCurrentValue = it;
					this->idHovered = -1;
					this->bIsActive = false;
					return true;
				}
			}
		}
		else
			this->idHovered = -1;
	}
	}

	return false;
}


void ComboBox::SetupPositions()
{
	this->SetupBaseSize();

	this->rcSelectable = [this]()
	{
		int posy = 18;
		return SRect(this->rcBoundingBox.left, posy, this->rcBoundingBox.right, posy + pFont->iHeight + int(float(style.iPaddingY) * 0.5f + 3));
	}();
}


SPoint ComboBox::GetSelectableSize()
{
	SPoint vecTmpSize;
	vecTmpSize.y = pFont->iHeight + int(float(6) * 0.5f);
	vecTmpSize.x = this->GetSize().x;
	return vecTmpSize;
}


void ComboBox::RenderSelectables()
{
	/* Background square for the list */
	g_Render.RectFilled({ this->rcBoundingBox.left, this->rcBoundingBox.bottom + 3 },
		{ this->rcBoundingBox.right, this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * int(this->vecSelectables.size()) },
		Color(35, 35, 35, g_Settings.iAlpha));

	/* Highlight selectable if its selected (-1 = no hovered one) */
	if (this->idHovered != -1)
	{
		const auto rcElementBounds = [this]() -> SRect
		{
			int posy = this->rcBoundingBox.bottom + 1 + this->rcBoundingBox.Height() * this->idHovered;
			return {
				this->rcBoundingBox.left + 1,
				posy + 1,
				this->rcBoundingBox.right - 1,
				posy + this->rcBoundingBox.Height() - 1
			};
		};

		g_Render.RectFilled(rcElementBounds(), Color(25, 25, 25, g_Settings.iAlpha));
	}

	/* Render all of the selectable labels (names) in the middle */
	auto index = 0;
	const auto ptMid = this->rcBoundingBox.Button();
	for (const auto& it : vecSelectables) {
		g_Render.String({ ptMid.x, ptMid.y + this->rcBoundingBox.Height() * (index++ + 1) + 1 },
			CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, Color(158, 158, 158, g_Settings.iAlpha), pFont.get(), it.c_str());
	}

	if (*this->iCurrentValue != -1)
	{
		g_Render.String({ this->rcBoundingBox.left + 10, this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * *this->iCurrentValue + 10 },
			CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, g_Settings.Miscellaneous.cMenuColor, pFont.get(), this->vecSelectables[*this->iCurrentValue].c_str());
	}
}

MultiBox::MultiBox(const std::string& strLabel, std::vector<std::string> vecBoxOptions, bool* bEnabled, ObjectPtr pParent)
{
	this->type = TYPE_MULTI;
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->idHovered = -1;
	this->bEnabled = bEnabled;
	this->vecSelectables = vecBoxOptions;
}


void MultiBox::Initialize()
{
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 18;
}


void MultiBox::Render()
{
	std::string format;

	for (int i = 0; i < vecSelectables.size(); i++)
	{
		auto formatLen = format.length() < 20;

		auto first = format.length() <= 0;
		if ((bEnabled[i]) && formatLen)
		{
			if (!first) {
				format.append(", ");
			}

			format.append(vecSelectables.at(i));
		}
		else if (!formatLen) {
			format.append("...");
			break;
		}
	}

	if (format.length() <= 0) {
		format += "";
	}

	/* Render the selectable with the value in the middle */
	g_Render.RectFilled(this->rcBoundingBox, Color(39, 39, 39, g_Settings.iAlpha));
	g_Render.String(this->rcBoundingBox.Button(), CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, Color(160, 160, 160, g_Settings.iAlpha), pFont.get(), format.c_str());

	/* Render the selectable with the value in the middle and highlight if hovered */
	if (this->bIsHovered) {
		g_Render.RectFilled(this->rcBoundingBox, style.colHover);
	}

	/* Render the small triangle */
	{
		SPoint ptPosMid, ptPosLeft, ptPosRight;

		/* Draw two different versions (top-down, down-top) depending on activation */
		if (!this->bIsActive)
		{
			ptPosMid.x = this->rcBoundingBox.right - 8;
			ptPosRight.x = this->rcBoundingBox.right - 5;
			ptPosLeft.x = this->rcBoundingBox.right - 10;

			ptPosRight.y = ptPosLeft.y = this->rcBoundingBox.top + 8;
			ptPosMid.y = this->rcBoundingBox.bottom - 7;
		}
		else
		{
			ptPosMid.x = this->rcBoundingBox.right - 8;
			ptPosRight.x = this->rcBoundingBox.right - 5;
			ptPosLeft.x = this->rcBoundingBox.right - 11;

			ptPosRight.y = ptPosLeft.y = this->rcBoundingBox.bottom - 7;
			ptPosMid.y = this->rcBoundingBox.top + 7;
		}

		g_Render.TriangleFilled(ptPosLeft, ptPosRight, ptPosMid, Color(162, 162, 162, g_Settings.iAlpha));
	}/*-------------------------*/

	/* Rectangle of the combo selectables, for scissorrect */
	const SRect vpCombo =
	{
		rcBoundingBox.left,
		rcBoundingBox.bottom,
		rcBoundingBox.right,
		rcBoundingBox.bottom + rcBoundingBox.Height() * int(vecSelectables.size())
	};

	/* Render selectables only within the rect, useful for scroll usage (if implemented later on) */
	g_Render.SetCustomScissorRect(vpCombo);
	if (this->bIsActive)
		RenderSelectables();
	g_Render.RestoreOriginalScissorRect();

	g_Render.Rect(this->rcBoundingBox, style.colSectionOutl(g_Settings.iAlpha));

	if (bIsActive) {
		g_Render.Rect({ this->rcBoundingBox.left, this->rcBoundingBox.bottom + 3 },
			{ this->rcBoundingBox.right, this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * int(this->vecSelectables.size()) },
			style.colSectionOutl(g_Settings.iAlpha));
	}
}


bool MultiBox::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	{
		/* If the main selectabled is hovered, handle only its input */
		if (this->bIsHovered)
		{
			this->idHovered = -1;
			if (mouseCursor->bLMBPressed)
			{
				this->bIsActive = !bIsActive;
				/* If active, request focus so its rendered on top of every other selectable */
				if (this->bIsActive)
					this->RequestFocus();
				return true;
			}
		}
		else if (this->bIsActive &&
			mouseCursor->IsInBounds({ this->rcBoundingBox.left, this->rcBoundingBox.bottom, this->rcBoundingBox.right,
									  this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * int(this->vecSelectables.size()) }))
		{
			for (std::size_t it = 0; it < this->vecSelectables.size(); ++it)
			{
				/* Bounds of the looped element. */
				const auto rcElementBounds = [this, it]()
				{
					int posy = this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * it;
					return SRect(this->rcBoundingBox.left, posy, this->rcBoundingBox.right, posy + this->rcBoundingBox.Height());
				};

				/* If we don't hover the element - ignore */
				if (!mouseCursor->IsInBounds(rcElementBounds()))
					continue;

				this->idHovered = it;
				if (mouseCursor->bLMBPressed)
				{
					this->bEnabled[it] = !this->bEnabled[it];
					this->idHovered = -1;
					//this->bIsActive = false;
					return true;
				}
			}
		}
		else
			this->idHovered = -1;
	}
	}

	return false;
}


void MultiBox::SetupPositions()
{
	this->SetupBaseSize();

	this->rcSelectable = [this]()
	{
		int posy = this->rcBoundingBox.top + pFont->iHeight + int(float(style.iPaddingY) * 0.5f);
		return SRect(this->rcBoundingBox.left, posy, this->rcBoundingBox.right, posy + pFont->iHeight + int(float(style.iPaddingY) * 0.5f + 3));
	}();
}


SPoint MultiBox::GetSelectableSize()
{
	SPoint vecTmpSize;
	vecTmpSize.y = pFont->iHeight + int(float(style.iPaddingY) * 0.5f);
	vecTmpSize.x = this->GetSize().x;
	return vecTmpSize;
}


void MultiBox::RenderSelectables()
{
	/* Background square for the list */
	g_Render.RectFilled({ this->rcBoundingBox.left, this->rcBoundingBox.bottom + 3 },
		{ this->rcBoundingBox.right, this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * int(this->vecSelectables.size()) },
		Color(35, 35, 35, g_Settings.iAlpha));

	/* Highlight selectable if its selected (-1 = no hovered one) */
	if (this->idHovered != -1)
	{
		const auto rcElementBounds = [this]() -> SRect
		{
			int posy = this->rcBoundingBox.bottom + 1 + this->rcBoundingBox.Height() * this->idHovered;
			return {
				this->rcBoundingBox.left + 1,
				posy + 1,
				this->rcBoundingBox.right - 1,
				posy + this->rcBoundingBox.Height() - 1
			};
		};

		g_Render.RectFilled(rcElementBounds(), Color(25, 25, 25, g_Settings.iAlpha));
	}

	/* Render all of the selectable labels (names) in the middle */

	auto index = 0;
	const auto ptMid = this->rcBoundingBox.Button();
	for (const auto& it : vecSelectables) {
		g_Render.String({ ptMid.x, ptMid.y + this->rcBoundingBox.Height() * (index++ + 1) + 1 },
			CD3DFONT_CENTERED_Y | CD3DFONT_DROPSHADOW, Color(158, 158, 158, g_Settings.iAlpha), pFont.get(), it.c_str());
	}

	for (int i = 0; i < vecSelectables.size(); i++) {
		if (!bEnabled[i]) continue;

		auto & current = vecSelectables.at(i);

		g_Render.String({ this->rcBoundingBox.left + 10, this->rcBoundingBox.bottom + this->rcBoundingBox.Height() * i + 10 },
			CD3DFONT_CENTERED_Y, g_Settings.Miscellaneous.cMenuColor, pFont.get(), current.c_str());
	}
}

template<typename T>
Slider<T>::Slider(const std::string& strLabel, std::string& strAddon, T* tValue, T tMinValue, T tMaxValue, ObjectPtr pParent)
{
	this->pParent = pParent;
	this->strLabel = strLabel;
	this->nValue = tValue;
	this->nMin = tMinValue;
	this->nMax = tMaxValue;
	this->type = TYPE_SLIDER;
	this->strAddon = strAddon;

}


template <typename T>
void Slider<T>::Initialize()
{
	this->szSizeObject.x = this->pParent->GetMaxChildWidth() - 56;
	this->szSizeObject.y = 7;
	this->szSelectableSize = { this->szSizeObject.x, pFont->iHeight + int(float(style.iPaddingY) - 11) };

	this->SetValue(*nValue); /* Since its limited, it should not be any higher - even when set in settings before. */
}


template<typename T>
void Slider<T>::Render()
{
	std::stringstream ssToRender;
	ssToRender << strLabel;
	std::stringstream ssToRender2;

	ssToRender2 << *this->nValue << strAddon;

	//g_Render.RectFilledGradient(this->rcMinus, g_Settings.Miscellaneous.cMenuColor, g_Settings.Miscellaneous.cMenuColor(120), GradientType::GRADIENT_VERTICAL);
	//g_Render.RectFilledGradient(this->rcPlus, g_Settings.Miscellaneous.cMenuColor, g_Settings.Miscellaneous.cMenuColor(120), GradientType::GRADIENT_VERTICAL);

	/* Render the selectable with the value in the middle */
	g_Render.RectFilledGradient(this->rcSelectable, Color(50, 50, 50, (g_Settings.iAlpha)), style.colComboBoxRect, GradientType::GRADIENT_VERTICAL);

	/* Fill the part of slider before the represented value */
	g_Render.RectFilled({ this->GetZeroPos(), this->rcSelectable.top + 1, this->iButtonPosX, this->rcSelectable.bottom - 1 },
		g_Settings.Miscellaneous.cMenuColor((g_Settings.iAlpha)));

	g_Render.RectFilledGradient({ this->GetZeroPos(), this->rcSelectable.top + 1, this->iButtonPosX, this->rcSelectable.bottom - 1 },
		Color::Black(0), Color(3, 3, 3, 100), GradientType::GRADIENT_VERTICAL);

	/* Render outline */
	g_Render.Rect(this->rcSelectable, style.colSectionOutl((g_Settings.iAlpha)));

	if (*this->nValue != this->nMin)
		g_Render.String(this->rcBoundingBox.left - 18, this->rcBoundingBox.top - 7, CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_X, style.colText(g_Settings.iAlpha), g_Fonts.pFontTahoma8.get(), "_");
	if (*this->nValue != this->nMax)
		g_Render.String(this->rcBoundingBox.right + 18, this->rcBoundingBox.top - 2, CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_X, style.colText(g_Settings.iAlpha), g_Fonts.pFontTahoma8.get(), "+");

	/* Renders the text */
	g_Render.String(this->iButtonPosX, this->rcSelectable.bottom - 8, CD3DFONT_DROPSHADOW | CD3DFONT_CENTERED_X, style.colText(g_Settings.iAlpha), g_Fonts.pFontTahoma8.get(), ssToRender2.str().c_str());

	if (bIsHovered) {
		g_Render.RectFilled({ this->GetZeroPos(), this->rcSelectable.top + 1, this->iButtonPosX, this->rcSelectable.bottom - 1 },
			style.colHover);
	}
}


template <typename T>
bool Slider<T>::UpdateData()
{
	if (!this->bIsVisible)
		return false;

	if (mouseCursor->IsInBounds(this->rcMinus))
	{
		if (mouseCursor->bLMBPressed)
		{
			this->SetValue(*this->nValue - static_cast<T>(1.f));
			return false;
		}
	}

	if (mouseCursor->IsInBounds(this->rcPlus))
	{
		if (mouseCursor->bLMBPressed)
		{
			this->SetValue(*this->nValue + static_cast<T>(1.f));
			return false;
		}
	}
}


template <typename T>
void Slider<T>::SetupPositions()
{
	this->SetupBaseSize();

	this->rcSelectable =
	{
		rcBoundingBox.left,
		rcBoundingBox.top,
		rcBoundingBox.right,
		rcBoundingBox.bottom
	};

	this->rcMinus = [this]()
	{
		auto size = int(float(rcBoundingBox.Height()) * 0.8f);
		auto diff = (this->rcBoundingBox.Height() - size) / 2;
		return SRect(rcBoundingBox.left + diff - 21, rcBoundingBox.top + diff, rcBoundingBox.left + diff + size - 21, rcBoundingBox.bottom - diff);
	}();

	this->rcPlus = [this]()
	{
		auto size = int(float(rcBoundingBox.Height()) * 0.8f);
		auto diff = (this->rcBoundingBox.Height() - size) / 2;
		return SRect(rcBoundingBox.right - diff + 16, rcBoundingBox.top + diff, rcBoundingBox.right - diff + size + 16, rcBoundingBox.bottom - diff);
	}();

	/* Update button position */
	this->iButtonPosX = (this->rcSelectable.left +
		static_cast<int>((*this->nValue - this->nMin) * float(this->rcBoundingBox.Width()) / (this->nMax - this->nMin)));
}


template <typename T>
bool Slider<T>::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	{
		/* Make sure we correct our hover state */
		this->bIsHovered = mouseCursor->IsInBounds(this->rcSelectable);

		if (uMsg != WM_LBUTTONDOWN)
			this->iDragX = mouseCursor->GetPos().x;
	}
	case WM_LBUTTONDOWN:
	{

		/* Make sure we correct our hover state */
		this->bIsHovered = mouseCursor->IsInBounds(this->rcSelectable);

		if (mouseCursor->bLMBPressed && bIsHovered)
			bPressed = true;
		else if (!mouseCursor->bLMBHeld && bPressed)
			bPressed = false;

		if (this->bPressed)
		{
			if (iDragX == 0)
				this->iDragX = mouseCursor->GetPos().x;

			this->iDragOffset = this->iDragX - this->iButtonPosX;
			this->iDragX = mouseCursor->GetPos().x;

			if (this->iDragOffset != 0)
			{
				this->RequestFocus();
				this->SetValue(static_cast<T>((this->iDragOffset * this->GetValuePerPixel()) + *this->nValue));
				return true;
			}
		}
		break;

	}
	}

	return false;
}


template <typename T>
bool Slider<T>::HandleKeyboardInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_HOME:
			this->SetValue(nMin);
			return true;
		case VK_END:
			this->SetValue(nMax);
			return true;
		case VK_LEFT:
		case VK_DOWN:
			this->SetValue(*this->nValue - static_cast<T>(this->GetValuePerPixel()));
			return true;
		case VK_RIGHT:
		case VK_UP:
			this->SetValue(*this->nValue + static_cast<T>(this->GetValuePerPixel()));
			return true;
		case VK_NEXT:
			this->SetValue(*this->nValue - static_cast<T>(1.f));
			return true;
		case VK_PRIOR:
			this->SetValue(*this->nValue + static_cast<T>(1.f));
			return true;
		}
	}
	}

	return false;
}


template<typename T>
float Slider<T>::GetValuePerPixel() const
{
	return float(this->nMax - this->nMin) / this->szSizeObject.x;
}


template<typename T>
void Slider<T>::SetValue(T flValue)
{
	flValue = max(flValue, this->nMin);
	flValue = min(flValue, this->nMax);

	*this->nValue = flValue;
	this->SetupPositions();
}


template <typename T>
int Slider<T>::GetZeroPos()
{
	if (this->nMin < 0)
		return this->rcSelectable.left + static_cast<int>((0 - this->nMin) * float(this->rcBoundingBox.Width()) / float(this->nMax - this->nMin));

	return this->rcSelectable.left + 1;
}

ColorPicker::ColorPicker(const std::string& strLabelName, Color* colorOutput, ObjectPtr pParent)
{
	this->pParent = pParent;
	this->strLabel = strLabelName;

	this->iPickerSize = 170;
	this->iHueBarSize = 10;

	this->colOutput = colorOutput;

	this->type = TYPE_COLOR;

	colOutput->ToHSV(colHSV[0], colHSV[1], colHSV[2]);
}

void ColorPicker::Initialize()
{
	this->szSizeObject.y = -6;

	this->szColorPicker.y = 6;
	this->szColorPicker.x = this->pParent->GetMaxChildWidth() - 56;
}

void ColorPicker::Render()
{
	/* Render picker rect */
	g_Render.RectFilled(this->rcSelectable, *this->colOutput);

	if (mouseCursor->IsInBounds(rcSelectable)) {

		g_Render.RectFilled(this->rcSelectable, style.colHover);
	}

	if (this->bIsActive)
	{
		/* Render sub-window for picker */
		g_Render.RectFilled(rcWindowBounds, Color(40, 40, 40, (g_Settings.iAlpha)));
		g_Render.Rect(rcWindowBounds, Color(2, 2, 2, (g_Settings.iAlpha)));

		static Color hueColors[7] =
		{
			{ 255, 0, 0   },
			{ 255, 255, 0 },
			{ 0, 255, 0   },
			{ 0, 255, 255 },
			{ 0, 0, 255   },
			{ 255, 0, 255 },
			{ 255, 0, 0   }
		};

		/* Render hue bar */
		for (auto i = 0; i < 6; ++i)
		{
			const SRect tmpHueRect =
			{
				rcHueBar.left,
				int(rcHueBar.top + i * (float(rcPicker.Height()) / 6.f)),
				rcHueBar.right,
				int(rcHueBar.top + (i + 1) * (float(rcPicker.Height()) / 6.f))
			};

			g_Render.RectFilledGradient(tmpHueRect, hueColors[i], hueColors[i + 1], GradientType::GRADIENT_VERTICAL);
			g_Render.Rect(this->rcHueBar, Color::Black(g_Settings.iAlpha));
		}

		Color Alpha;
		Alpha = *this->colOutput;

		/* Render alpha bar */
		g_Render.RectFilledGradient(this->rcAlphaBar, Color(0, 0, 0, 0), Alpha(g_Settings.iAlpha), GradientType::GRADIENT_HORIZONTAL);
		g_Render.Rect(this->rcAlphaBar, Color::Black(g_Settings.iAlpha));

		/* Render picker itself */
		Color picker;
		picker.FromHSV(colHSV[0], 1, 1);
		g_Render.RectFilledGradientMultiColor(this->rcPicker, Color::White(g_Settings.iAlpha), picker, Color::White(g_Settings.iAlpha), picker);
		g_Render.RectFilledGradientMultiColor(this->rcPicker, Color::Black(0), Color::Black(0), Color::Black(g_Settings.iAlpha), Color::Black(g_Settings.iAlpha));
		g_Render.Rect(this->rcPicker, Color::Black(g_Settings.iAlpha));

		/* Render hue bar position indicator */
		g_Render.RectFilled(ptHueBarMousePos.x - 2 - MulInt(rcHueBar.Width(), 0.5f), ptHueBarMousePos.y - 2,
			ptHueBarMousePos.x + 2 + MulInt(rcHueBar.Width(), 0.5f), ptHueBarMousePos.y + 2,
			Color::White(g_Settings.iAlpha));
		g_Render.Rect(ptHueBarMousePos.x - 2 - MulInt(rcHueBar.Width(), 0.5f), ptHueBarMousePos.y - 2,
			ptHueBarMousePos.x + 2 + MulInt(rcHueBar.Width(), 0.5f), ptHueBarMousePos.y + 2,
			Color::Black(g_Settings.iAlpha));

		/* Render alpha bar position indicator */
		g_Render.RectFilled(ptAlphaMousePos.x - 2, ptAlphaMousePos.y - 2 - MulInt(rcAlphaBar.Height(), 0.5f),
			ptAlphaMousePos.x + 2, ptAlphaMousePos.y + 2 + MulInt(rcAlphaBar.Height(), 0.5f),
			Color::White(g_Settings.iAlpha));
		g_Render.Rect(ptAlphaMousePos.x - 2, ptAlphaMousePos.y - 2 - MulInt(rcAlphaBar.Height(), 0.5f),
			ptAlphaMousePos.x + 2, ptAlphaMousePos.y + 2 + MulInt(rcAlphaBar.Height(), 0.5f),
			Color::Black(g_Settings.iAlpha));

		/* Render picker color position */
		g_Render.RectFilled(ptPickerMousePos.x - 2, ptPickerMousePos.y - 2, ptPickerMousePos.x + 2, ptPickerMousePos.y + 2, Color::White(g_Settings.iAlpha));
		g_Render.Rect(ptPickerMousePos.x - 2, ptPickerMousePos.y - 2, ptPickerMousePos.x + 2, ptPickerMousePos.y + 2, Color::Black(g_Settings.iAlpha));
	}

	g_Render.RectFilledGradient(this->rcSelectable, Color(0, 0, 0, 0), Color(3, 3, 3, 100), GradientType::GRADIENT_VERTICAL);

	/* Outline */
	g_Render.Rect(this->rcSelectable, style.colSectionOutl(g_Settings.iAlpha));
}

/* Called in WndProc hook. Returns true/false depending if we did or didnt capture input. */
bool ColorPicker::HandleMouseInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this->bIsVisible)
		return false;

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	{
		/* Hovered state */
		if (mouseCursor->IsInBounds(rcSelectable))
		{
			if (mouseCursor->bLMBPressed)
			{
				this->bIsActive = !this->bIsActive;
				this->RequestFocus();
				return true;
			}
		}
		else if (!mouseCursor->IsInBounds(rcAlphaBar) && !mouseCursor->IsInBounds(rcHueBar) && !mouseCursor->IsInBounds(rcPicker) && !mouseCursor->IsInBounds(rcWindowBounds))
		{
			if (mouseCursor->bLMBPressed)
				this->bIsActive = false;
		}

		/* Static as you still wouldnt have 2 pickers open at the same time & makes it easier */
		static int  iPickerID = -1;

		if (this->bIsActive)
		{
			this->rcBoundingBox.right = this->rcWindowBounds.right;
			this->rcBoundingBox.bottom = this->rcWindowBounds.bottom;
			if (mouseCursor->IsInBounds(this->rcWindowBounds))
			{
				/* Take over window controls and make color picker draw on top (look at the hint)*/

				/* React only if we start dragging in bounds (first wm_lbuttondown tick) */
				if (mouseCursor->bLMBPressed)
				{
					if (mouseCursor->IsInBounds(rcPicker))
						iPickerID = 0;
					else if (mouseCursor->IsInBounds(rcHueBar))
						iPickerID = 1;
					else if (mouseCursor->IsInBounds(rcAlphaBar))
						iPickerID = 2;
					else
						iPickerID = -1;
				}
			}
			/* Wm_lbuttondown is not pressed / held */
			if (!mouseCursor->bLMBHeld)
				iPickerID = -1;


			if (iPickerID != -1)
			{
				/* Make sure we maintain focus */
				/* Save actual mouse position depending on the picker used */
				switch (iPickerID)
				{
				case 0:
					ptPickerMousePos = { std::clamp(GET_X_LPARAM(lParam), rcPicker.left, rcPicker.right),
										 std::clamp(GET_Y_LPARAM(lParam), rcPicker.top, rcPicker.bottom) };
					break;
				case 1:
					ptHueBarMousePos = { rcHueBar.Mid().x,
										 std::clamp(GET_Y_LPARAM(lParam), rcPicker.top, rcPicker.bottom) };
					break;
				case 2:
					ptAlphaMousePos = { std::clamp(GET_X_LPARAM(lParam), rcPicker.left, rcPicker.right),
										rcAlphaBar.Mid().y };
					break;
				}

				/* Calculate hsv values (easier than rgb) */
				colHSV[0] = std::clamp(float(ptHueBarMousePos.y - rcPicker.top) / float(rcPicker.Height() - 1), 0.0f, 1.0f);
				colHSV[1] = std::clamp(float(ptPickerMousePos.x - rcPicker.left) / float(rcPicker.Width() - 1), 0.0f, 1.0f);
				colHSV[2] = 1.0f - std::clamp(float(ptPickerMousePos.y - rcPicker.top) / float(rcPicker.Height() - 1), 0.0f, 1.0f);

				/* Save picked value to the color */
				this->colOutput->FromHSV(colHSV[0], colHSV[1], colHSV[2]);
				this->colOutput->alpha = int(float(ptAlphaMousePos.x - rcAlphaBar.left) / rcAlphaBar.Width() * 255.f);
			}

			return iPickerID != -1;
		}
	}
	}

	return false;
}

bool ColorPicker::UpdateData()
{
	if (mouseCursor->IsInBounds(rcSelectable) && GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('C'))
	{
		Color col = *this->colOutput;
		std::stringstream color_hex;

		color_hex << "#";
		color_hex << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << +col.red;
		color_hex << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << +col.green;
		color_hex << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << +col.blue;
		color_hex << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << +col.alpha;


		if (OpenClipboard(nullptr))
		{
			EmptyClipboard();
			HGLOBAL clipboard_buffer = GlobalAlloc(GMEM_DDESHARE, color_hex.str().size() + 1);
			char *buffer = (char*)GlobalLock(clipboard_buffer);
			strcpy(buffer, color_hex.str().c_str());

			GlobalUnlock(clipboard_buffer);
			SetClipboardData(CF_TEXT, clipboard_buffer);
			CloseClipboard();
		}
	}
	else if (mouseCursor->IsInBounds(rcSelectable) && GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('V'))
	{
		if (OpenClipboard(nullptr))
		{
			std::string input((char*)GetClipboardData(CF_TEXT));

			/*input.erase(input.begin(), std::find_if(input.begin(), input.end(), [](int ch) {
				return !std::isspace(ch);
				}));
			input.erase(std::find_if(input.rbegin(), input.rend(), [](int ch) {
				return !std::isspace(ch);
				}).base(), input.end());*/

			if (input.at(0) != '#')
				return false;

			int component_r = std::stoi(input.substr(1, 2), 0, 16);
			int component_g = std::stoi(input.substr(3, 2), 0, 16);
			int component_b = std::stoi(input.substr(5, 2), 0, 16);
			int component_a = input.size() > 7 ? std::stoi(input.substr(7, 2), 0, 16) : 255;

			Color colPaste = Color(component_r, component_g, component_b, component_a);

			*this->colOutput = colPaste;

			CloseClipboard();
		}
	}

	return false;
}

/* Called once for initialization and on gui window drag */
void ColorPicker::SetupPositions()
{
	this->rcBoundingBox.right = this->rcBoundingBox.left + this->szColorPicker.x;
	this->rcBoundingBox.bottom = this->rcBoundingBox.top + this->szColorPicker.y;

	this->rcSelectable =
	{
		this->rcBoundingBox.right - 22,
		this->rcBoundingBox.top + MulInt(this->rcBoundingBox.Height(), 0.f),
		this->rcBoundingBox.right - 0,
		this->rcBoundingBox.top + MulInt(this->rcBoundingBox.Height(), 1.25f)
	};
	this->rcWindowBounds =
	{
		this->rcBoundingBox.right - 22,
		this->rcBoundingBox.bottom + 3,
		this->rcBoundingBox.right + this->iPickerSize - 22 + this->iHueBarSize + 6 * 3,
		this->rcBoundingBox.bottom + this->iPickerSize + 3 + this->iHueBarSize + 6 * 3,
	};
	this->rcPicker =
	{
		this->rcWindowBounds.left + 6,
		this->rcWindowBounds.top + 6,
		this->rcWindowBounds.left + 6 + this->iPickerSize,
		this->rcWindowBounds.top + 6 + this->iPickerSize
	};
	this->rcHueBar =
	{
		this->rcPicker.right + 6,
		this->rcPicker.top,
		this->rcPicker.right + 6 + this->iHueBarSize,
		this->rcPicker.bottom
	};
	this->rcAlphaBar =
	{
		this->rcPicker.left,
		this->rcPicker.bottom + 6,
		this->rcPicker.right,
		this->rcPicker.bottom + 6 + this->iHueBarSize
	};


	ptHueBarMousePos = { rcHueBar.Mid().x, std::clamp(rcHueBar.top + int(std::roundf(colHSV[0] * rcHueBar.Height())), rcHueBar.top, rcHueBar.bottom) };

	ptAlphaMousePos = { rcAlphaBar.left + int(std::roundf(colOutput->alpha / 255.f * rcAlphaBar.Width())), rcAlphaBar.Mid().y };

	ptPickerMousePos = { std::clamp(rcPicker.left + int(std::roundf(colHSV[1] * rcPicker.Width())), rcPicker.left, rcPicker.right),
						 std::clamp(rcPicker.top + int(std::roundf((1.f - colHSV[2]) * rcPicker.Height())), rcPicker.top, rcPicker.bottom) };

}