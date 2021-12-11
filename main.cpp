
#define _CRT_SECURE_NO_WARNINGS

#ifdef _DEBUG
//#include <vld.h>
#endif


#include "mimalloc-new-delete.h"
#include <Windows.h>

#include <string>
#include <algorithm>


#include "claujson.h"

#include "smart_ptr.h"


#include <utility>
//
//

#define __WXMSW__

#include <wx/wx.h>

#include <wx/defs.h>
//
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/dataview.h>
#include <wx/stc/stc.h>
#include <wx/frame.h>


namespace wiz {

	class claujson::Data Parse(const std::string& str) {
		class claujson::Data data;
	
		if (claujson::Parse_One(str, data) == 0) {
			// success
		}
		else { // fail
			// error
		}
		

		return data;
	}

	std::string ToString(const claujson::Data& data) {
		std::string result;

		switch (data.type) {
			case simdjson::internal::tape_type::STRING:
			{
				result = "\"";
				result += data.str_val;
				result += "\"";
			}
			break;

			case simdjson::internal::tape_type::DOUBLE:
				result = std::to_string(data.float_val);
				break;
			case simdjson::internal::tape_type::INT64:
				result = std::to_string(data.int_val);
				break;
			case simdjson::internal::tape_type::UINT64:
				result = std::to_string(data.uint_val);
				break;
			case simdjson::internal::tape_type::TRUE_VALUE:
				result = "TRUE";
				break;
			case simdjson::internal::tape_type::FALSE_VALUE:
				result = "FALSE";
				break;
			case simdjson::internal::tape_type::NULL_VALUE:
				result = "NULL";
				break;
		}

		return result;
	}
	
	inline int64_t GetIdx(claujson::UserType* ut, int64_t position, int64_t start) {
		int64_t idx = start;

		if (ut->is_object()) {
			for (int64_t i = 0; start + i < ut->get_data_size() && i < position; ++i) {
				++idx;
				if (ut->get_data_list(start + i)->is_item_type()) {
					++idx;
				}
			}
		}
		else {
			return start + position;
		}

		return idx;
	}

	template <class T>
	T Max(const T& x, const T& y) {
		return x > y ? x : y;
	}

	std::string _ToStringEx(claujson::UserType* ut, long long start, long long& count, long long count_limit) {

		if (count >= count_limit) {
			return std::string();
		}

		std::string str;

		// key = data, key = { }, data, { }, ...
		for (int i = start; i < ut->get_data_size() && count < count_limit; ++i) {
			++count;

			if (ut->get_data_list(i)->get_value().is_key) {
				str += "\"";
				str += ut->get_data_list(i)->get_value().str_val;
				str += "\" : ";
			}

			if (ut->get_data_list(i)->is_object()) {
				str += " { ";

				str += _ToStringEx(ut->get_data_list(i), 0, count, count_limit);
				if (ut->get_data_list(i)->is_object()) {
					str += " } ";
				}
				else {
					str += " ] ";
				}
			}
			else if(ut->get_data_list(i)->is_array()) {
				str += " [ ";

				str += _ToStringEx(ut->get_data_list(i), 0, count, count_limit);
				if (ut->get_data_list(i)->is_object()) {
					str += " } ";
				}
				else {
					str += " ] ";
				}
			}
			else {
				// key = data.
				if (ut->is_object()) {
					str += ToString(ut->get_data_list(i + 1)->get_value());
					++i;
				}
				else {
					str += ToString(ut->get_data_list(i)->get_value());
				}
			}

			if (i < ut->get_data_size()) {
				str += " , ";
			}

			str += "\n";
		}

		return str;
	}

	std::string ToStringEx(claujson::UserType* ut, long long start) {
		long long count = 0;
		return _ToStringEx(ut, start, count, 256);
	}
}


enum class Encoding {
	ANSI, UTF8
};
Encoding encoding = Encoding::UTF8;

//auto default_cp = GetConsoleCP();

inline std::string Convert(const wxString& str) {
	if (Encoding::UTF8 == encoding) {
		return str.ToUTF8().data();
	}
	else {
		return str.ToStdString();
	}
}
inline std::string Convert(wxString&& str) {
	if (Encoding::UTF8 == encoding) {
		return str.ToUTF8().data();
	}
	else {
		return str.ToStdString();
	}
}

 wxString Convert(const std::string& str) {
	if (Encoding::UTF8 == encoding) {

		

		return wxString(str.c_str(), wxConvUTF8);
	}
	else {
		//?
		wxString temp(str.c_str(), wxCSConv(wxFONTENCODING_SYSTEM));

		if (!str.empty() && temp.empty()) {
			temp = wxString(str.c_str(), wxConvISO8859_1);
		}

		return temp;
	}
}
 wxString Convert(std::string&& str) {
	if (Encoding::UTF8 == encoding) {
		return wxString(str.c_str(), wxConvUTF8);
	}
	else {

		wxString temp(str.c_str(), wxCSConv(wxFONTENCODING_SYSTEM));

		if (!str.empty() && temp.empty()) {
			temp = wxString(str.c_str(), wxConvISO8859_1);
		}

		return temp;
	}
}

///////////////////////////////////////////////////////////////////////////
#define NK_ENTER 13
#define NK_BACKSPACE 8

using namespace std;

class LangFrame : public wxFrame
{
private:

protected:

	wxStyledTextCtrl* m_code;
	wxButton* m_code_run_button;

	claujson::UserType** now;
	int* ptr_dataViewListCtrlNo;
	long long* ptr_position;

	wxDataViewListCtrl* m_dataViewListCtrl[4];

	// Virtual event handlers, overide them in your derived class

	virtual void m_code_run_buttonOnButtonClick(wxCommandEvent& event) {

		try {


			//RefreshTable(now);
		}
		catch (...) {
			//	RefreshTable(now);
				//
		}
	}


public:

	LangFrame(claujson::UserType** now, int* dataViewListCtrlNo,
		long long *position, wxDataViewListCtrl* m_dataViewListCtrl1, wxDataViewListCtrl* m_dataViewListCtrl2,
		wxDataViewListCtrl* m_dataViewListCtrl3, wxDataViewListCtrl* m_dataViewListCtrl4, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(770, 381), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

	~LangFrame();

};

LangFrame::LangFrame(claujson::UserType** now, int* dataViewListCtrlNo,
	long long* position, wxDataViewListCtrl* m_dataViewListCtrl1, wxDataViewListCtrl* m_dataViewListCtrl2,
	wxDataViewListCtrl* m_dataViewListCtrl3, wxDataViewListCtrl* m_dataViewListCtrl4, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style),
	 ptr_dataViewListCtrlNo(dataViewListCtrlNo), ptr_position(position)
{
	m_dataViewListCtrl[0] = m_dataViewListCtrl1;
	m_dataViewListCtrl[1] = m_dataViewListCtrl2;
	m_dataViewListCtrl[2] = m_dataViewListCtrl3;
	m_dataViewListCtrl[3] = m_dataViewListCtrl4;

	this->now = now;
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer;
	bSizer = new wxBoxSizer(wxVERTICAL);

	m_code = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);
	m_code->SetText(wxString(wxT(
		"#SimdclaujsonJson Explorer (https://github.com/vztpv/SimdclaujsonJsonExplorer) \n#		Á¦ÀÛÀÚ vztpv@naver.com\n"), wxConvUTF8));
	m_code->SetUseTabs(true);
	m_code->SetTabWidth(4);
	m_code->SetIndent(4);
	m_code->SetTabIndents(true);
	m_code->SetBackSpaceUnIndents(true);
	m_code->SetViewEOL(false);
	m_code->SetViewWhiteSpace(false);
	m_code->SetMarginWidth(2, 0);
	m_code->SetIndentationGuides(true);
	m_code->SetMarginType(1, wxSTC_MARGIN_SYMBOL);
	m_code->SetMarginMask(1, wxSTC_MASK_FOLDERS);
	m_code->SetMarginWidth(1, 16);
	m_code->SetMarginSensitive(1, true);
	m_code->SetProperty(wxT("fold"), wxT("1"));
	m_code->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
	m_code->SetMarginType(0, wxSTC_MARGIN_NUMBER);
	m_code->SetMarginWidth(0, m_code->TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
	m_code->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS);
	m_code->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("BLACK")));
	m_code->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, wxColour(wxT("WHITE")));
	m_code->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS);
	m_code->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("BLACK")));
	m_code->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("WHITE")));
	m_code->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
	m_code->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS);
	m_code->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("BLACK")));
	m_code->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("WHITE")));
	m_code->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS);
	m_code->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("BLACK")));
	m_code->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("WHITE")));
	m_code->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
	m_code->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
	m_code->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
	m_code->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));

	bSizer->Add(m_code, 9, wxEXPAND | wxALL, 5);

	//m_code->Hide();

	m_code_run_button = new wxButton(this, wxID_ANY, wxT("Run"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer->Add(m_code_run_button, 1, wxALL | wxEXPAND, 5);


	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);

	// Connect Events

	m_code_run_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LangFrame::m_code_run_buttonOnButtonClick), NULL, this);
}

LangFrame::~LangFrame()
{
	// Disconnect Events
	m_code_run_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LangFrame::m_code_run_buttonOnButtonClick), NULL, this);

}

class ChangeWindow : public wxDialog
{
private:
	wiz::SmartPtr<claujson::UserType> ut;
	bool isUserType; // ut(true) or it(false)
	int idx; // ut-idx or it-idx. or ilist idx(type == insert)
	int type; // change 1, insert 2
protected:
	wxTextCtrl* var_text;
	wxTextCtrl* val_text;
	wxButton* ok;
	claujson::PoolManager* manager;

	// Virtual event handlers, overide them in your derived class
	virtual void okOnButtonClick(wxCommandEvent& event) {
		string var(Convert(var_text->GetValue()));
		string val(Convert(val_text->GetValue()));

		try {
			if (1 == type && !isUserType) { // change
				claujson::Data x = wiz::Parse(var);
				claujson::Data y = wiz::Parse(val);


				if (x.type == simdjson::internal::tape_type::STRING) {
					x.is_key = true;
				}

				if (y.type == simdjson::internal::tape_type::ROOT) {
					return; // error
				}

				if (x.is_key && ut->is_object() && y.type != simdjson::internal::tape_type::START_ARRAY && y.type != simdjson::internal::tape_type::START_OBJECT) {
					if (idx != -1) {
						ut->get_data_list(idx)->set_value(x);
						ut->get_data_list(idx + 1)->set_value(y);
					}
				}
				else if (x.type == simdjson::internal::tape_type::ROOT && ut->is_array() && y.type != simdjson::internal::tape_type::START_ARRAY
					&& y.type != simdjson::internal::tape_type::START_OBJECT) {
					if (idx != -1) {
						ut->get_data_list(idx)->set_value(y);
					}
				}
				else {
					return; // error
				}
			}
			else if (1 == type) { // change && isUserType
				claujson::Data x = wiz::Parse(var);
				if (x.type == simdjson::internal::tape_type::STRING) {
					x.is_key = true;
				}

				if (x.is_key && ut->is_object()) {
					ut->get_data_list(idx)->set_value(x.str_val);
				}
				else if (x.type == simdjson::internal::tape_type::ROOT && ut->is_array()) {
					return;
				}
			}
			else if (2 == type) { // insert
				claujson::Data x = wiz::Parse(var);
				claujson::Data y = wiz::Parse(val);

				if (x.type == simdjson::internal::tape_type::STRING) {
					x.is_key = true;
				}

				if (y.type == simdjson::internal::tape_type::ROOT) {// ROOT as NONE
					return; // error
				}

				if (x.is_key && ut->is_object() && y.type != simdjson::internal::tape_type::START_ARRAY && y.type != simdjson::internal::tape_type::START_OBJECT) {
					ut->add_object_element(*manager, x, y);
				}
				else if (x.is_key && ut->is_object() && (y.type == simdjson::internal::tape_type::START_ARRAY || y.type == simdjson::internal::tape_type::START_OBJECT)) {
					if (y.type == simdjson::internal::tape_type::START_OBJECT) {
						claujson::UserType* object = claujson::UserType::make_object(*manager, x);
						ut->add_object_with_key(object);
					}
					else {
						claujson::UserType* _array = claujson::UserType::make_array(*manager, x);
						ut->add_array_with_key(_array);
					}
				}
				else if (x.type == simdjson::internal::tape_type::ROOT && (ut->is_array() || ut->is_root()) && y.type != simdjson::internal::tape_type::START_ARRAY && y.type != simdjson::internal::tape_type::START_OBJECT) {
					ut->add_array_element(*manager, y);
				}
				else if (x.type == simdjson::internal::tape_type::ROOT && (ut->is_array() || ut->is_root()) && (y.type == simdjson::internal::tape_type::START_ARRAY || y.type == simdjson::internal::tape_type::START_OBJECT)) {
					if (y.type == simdjson::internal::tape_type::START_OBJECT) {
						claujson::UserType* object = claujson::UserType::make_object(*manager, x);
						ut->add_object_with_no_key(object);
					}
					else {
						claujson::UserType* _array = claujson::UserType::make_array(*manager, x);
						ut->add_array_with_no_key(_array);
					}
				}
				else {
					return; // error
				}
			}
		}
		catch (...) {
			// print error to textbox?
			return;
		}

		Close();
	}

public:
	ChangeWindow(wxWindow* parent, claujson::PoolManager* manager, wiz::SmartPtr<claujson::UserType> ut, bool isUserType, int idx, int type, 
		wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(580, 198), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
	
	~ChangeWindow();
};

ChangeWindow::ChangeWindow(wxWindow* parent, claujson::PoolManager* manager, wiz::SmartPtr<claujson::UserType> ut, bool isUserType, int idx, int type, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: manager(manager), ut(ut), isUserType(isUserType), idx(idx), type(type),  wxDialog(parent, id, "change/insert window", pos, size, style)
{

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);

	var_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(var_text, 1, wxALL | wxEXPAND, 5);

	val_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(val_text, 1, wxALL | wxEXPAND, 5);

	ok = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer4->Add(ok, 0, wxALL | wxEXPAND, 5);

	if (1 == type) {
		if (isUserType) {
			var_text->SetValue(Convert(wiz::ToString(ut->get_data_list(idx)->get_value())));
		}
		else {
			var_text->SetValue(Convert(wiz::ToString(ut->get_data_list(idx)->get_value())));
			val_text->SetValue(Convert(wiz::ToString(ut->get_data_list(idx + 1)->get_value())));
		}
	}


	this->SetSizer(bSizer4);
	this->Layout();

	// Connect Events
	ok->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ChangeWindow::okOnButtonClick), NULL, this);
}

ChangeWindow::~ChangeWindow()
{
	// Disconnect Events
	ok->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ChangeWindow::okOnButtonClick), NULL, this);
}

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrame
///////////////////////////////////////////////////////////////////////////////
class MainFrame : public wxFrame
{
private:
	claujson::PoolManager manager;

	bool isMain = false;

	wiz::SmartPtr<claujson::UserType> global;
	claujson::UserType* now = nullptr;

	int dataViewListCtrlNo = -1;
	long long position = -1;

	std::vector<long long> part;
	const long long part_size = 1024;

	int run_count = 0;

	wiz::SmartPtr<bool> changed;

	std::vector<std::string> dir_vec;
private:
	void RefreshText(wiz::SmartPtr<claujson::UserType> now) {
		wxDataViewListCtrl* ctrl[4];
		ctrl[0] = m_dataViewListCtrl1;
		ctrl[1] = m_dataViewListCtrl2;
		ctrl[2] = m_dataViewListCtrl3;
		ctrl[3] = m_dataViewListCtrl4;

		long long start = 0;
		long long sum = part.back() * part_size;
		start = sum;

		textCtrl->ChangeValue(Convert(wiz::ToStringEx(now, start)));
	}

	void RefreshText2(wiz::SmartPtr<claujson::UserType> now) {
		wxDataViewListCtrl* ctrl[4];
		ctrl[0] = m_dataViewListCtrl1;
		ctrl[1] = m_dataViewListCtrl2;
		ctrl[2] = m_dataViewListCtrl3;
		ctrl[3] = m_dataViewListCtrl4;

		long long start = 0;
		long long sum = 0;
		long long idx = 0;

		for (int i = 0; i < dataViewListCtrlNo; ++i) {
			idx = wiz::GetIdx(now, ctrl[i]->GetItemCount(), idx);
		}
		sum += wiz::GetIdx(now, position, idx);
		sum += part.back() * part_size;
		start = sum;

		textCtrl->ChangeValue(Convert(wiz::ToStringEx(now, start)));
	}

	void changedEvent() {
		//m_code_run_result->ChangeValue(wxT("changed"));
		dir_text->SetLabelText(wxT(""));
		now = global;
		dir_vec.clear();
		dir_vec.push_back(".");
		position = -1;
		dataViewListCtrlNo = -1;

		part.clear();
		part.push_back(0);

		RefreshText(now);
		RefreshTable(now);
	
		*changed = false;
	}
	void RefreshTable(wiz::SmartPtr<claujson::UserType> now)
	{
		m_dataViewListCtrl1->DeleteAllItems();
		m_dataViewListCtrl2->DeleteAllItems();
		m_dataViewListCtrl3->DeleteAllItems();
		m_dataViewListCtrl4->DeleteAllItems();

		AddData(now, (part.back()) * part_size);

		dataViewListCtrlNo = -1;
		position = -1;

		{
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			for (int i = 0; i < 4; ++i) {
				if (ctrl[i]->GetItemCount() > 0) {
					dataViewListCtrlNo = i;
					position = 0;

					ctrl[dataViewListCtrlNo]->SelectRow(position);
					ctrl[dataViewListCtrlNo]->SetFocus();
					break;
				}
			}
		}


	}
	void AddData(wiz::SmartPtr<claujson::UserType> global, int start)
	{
		{
			int size = global->get_data_size();

			if (size > part_size) {
				if (size > part_size * part.back() && size <= part_size * (part.back() + 1)) {
					size = size - part_size * part.back();
				}
				else {
					size = part_size;
				}
			}

			int size_per_unit = size / 4;

			const int last_size = size;
			int count = start;

			wxVector<wxVariant> value;

			int i = 0;

			for ( ; i < size_per_unit; ++i) {
				value.clear();

				if (global->get_data_list(i)->is_user_type()) {
					if (wiz::ToString(global->get_data_list(i)->get_value()).empty()) {
						value.push_back(wxVariant(wxT("")));
					}
					else {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}
					if (global->get_data_list(i)->is_object()) {
						value.push_back(wxVariant(wxT("{}")));
					}
					else if (global->get_data_list(i)->is_array()) {
						value.push_back(wxVariant(wxT("[]")));
					}
					else {
						value.push_back(wxVariant(wxT("ERROR")));
					}
				}
				else { // data2
					if (global->is_object()) {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i + 1)->get_value()))));
						++i;
					}
					else {
						value.push_back(wxVariant(wxT("")));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}

				}

				m_dataViewListCtrl1->AppendItem(value);
				count++;
			}
			for ( ; i < size_per_unit * 2; ++i) {
				value.clear();

				if (global->get_data_list(i)->is_user_type()) {
					if (wiz::ToString(global->get_data_list(i)->get_value()).empty()) {
						value.push_back(wxVariant(wxT("")));
					}
					else {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}
					if (global->get_data_list(i)->is_object()) {
						value.push_back(wxVariant(wxT("{}")));
					}
					else if (global->get_data_list(i)->is_array()) {
						value.push_back(wxVariant(wxT("[]")));
					}
					else {
						value.push_back(wxVariant(wxT("ERROR")));
					}
				}
				else { // data2
					if (global->is_object()) {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i + 1)->get_value()))));
						++i;
					}
					else {
						value.push_back(wxVariant(wxT("")));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}

				}


				m_dataViewListCtrl2->AppendItem(value);
				count++;
			}
			for ( ; i < size_per_unit * 3; ++i) {
				value.clear();
				if (global->get_data_list(i)->is_user_type()) {
					if (wiz::ToString(global->get_data_list(i)->get_value()).empty()) {
						value.push_back(wxVariant(wxT("")));
					}
					else {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}
					if (global->get_data_list(i)->is_object()) {
						value.push_back(wxVariant(wxT("{}")));
					}
					else if (global->get_data_list(i)->is_array()) {
						value.push_back(wxVariant(wxT("[]")));
					}
					else {
						value.push_back(wxVariant(wxT("ERROR")));
					}
				}
				else { // data2
					if (global->is_object()) {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i + 1)->get_value()))));
						++i;
					}
					else {
						value.push_back(wxVariant(wxT("")));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}

				}


				m_dataViewListCtrl3->AppendItem(value);
				count++;
			}
			for ( ; i < last_size; ++i) {
				value.clear();

				if (global->get_data_list(i)->is_user_type()) {
					if (wiz::ToString(global->get_data_list(i)->get_value()).empty()) {
						value.push_back(wxVariant(wxT("")));
					}
					else {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}
					if (global->get_data_list(i)->is_object()) {
						value.push_back(wxVariant(wxT("{}")));
					}
					else if (global->get_data_list(i)->is_array()) {
						value.push_back(wxVariant(wxT("[]")));
					}
					else {
						value.push_back(wxVariant(wxT("ERROR")));
					}
				}
				else { // data2
					if (global->is_object()) {
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i + 1)->get_value()))));
						++i;
					}
					else {
						value.push_back(wxVariant(wxT("")));
						value.push_back(wxVariant(Convert(wiz::ToString(global->get_data_list(i)->get_value()))));
					}

				}


				m_dataViewListCtrl4->AppendItem(value);
				count++;
			}
		}
	}
protected:
	wxTextCtrl* textCtrl;
	
	wxMenuBar* menuBar;
	wxMenu* FileMenu;
	wxMenu* DoMenu;
	wxMenu* WindowMenu;
	wxButton* back_button; 
	wxButton* next;
	wxButton* before;
	wxTextCtrl* dir_text;
	wxButton* refresh_button;
	wxDataViewListCtrl* m_dataViewListCtrl1;
	wxDataViewListCtrl* m_dataViewListCtrl2;
	wxDataViewListCtrl* m_dataViewListCtrl3;
	wxDataViewListCtrl* m_dataViewListCtrl4;
	wxStatusBar* m_statusBar1;

	wxTextCtrl* m_code_run_result;
	wxStaticText* m_now_view_text;

	void EnterDir(const std::string& name, std::vector<long long>& part) {
		dir_vec.push_back(name);

		part.push_back(0);

		std::string dir = "";
		for (int i = 0; i < dir_vec.size(); ++i) {
			dir += "/";
			dir += dir_vec[i];


			dir += part[i] > 0 ?  std::string("part") + std::to_string(part[i]) : "";
		}

		dir_text->ChangeValue(Convert(dir));
	}
	void BackDir() {
		if (!dir_vec.empty()) {
			dir_vec.pop_back();

			std::string dir = "";
			for (int i = 0; i < dir_vec.size(); ++i) {
				dir += "/";
				dir += dir_vec[i];
				dir += part[i] > 0 ?  std::string("part") + std::to_string(part[i]) : "";
			}

			dir_text->ChangeValue(Convert(dir));
		}
	}
	// Virtual event handlers, overide them in your derived class
	virtual void FileLoadMenuOnMenuSelection(wxCommandEvent& event) {
		if (!isMain) { return; }
		wxFileDialog* openFileDialog = new wxFileDialog(this);
		static int count = 0;

		if (openFileDialog->ShowModal() == wxID_OK) {
			wxString _fileName = openFileDialog->GetPath();
			std::string fileName(_fileName.c_str());

			global->remove_all(manager);
			manager.Clear();

			claujson::UserType* x = nullptr;

			count++;

			std::vector<claujson::Block> blocks;

			if (nullptr != (x = claujson::Parse(fileName.c_str(), 0, global, blocks))) {
				encoding = Encoding::UTF8;
				//	SetConsoleOutputCP(65001); // for windows
					//setlocale(LC_ALL, "en_US.UTF-8");
				std::string temp = std::to_string(count);

				manager = claujson::PoolManager(x, std::move(blocks));
				
				m_code_run_result->ChangeValue(wxString::FromUTF8(temp.c_str()) + wxT("Load Success! file is UTF-8"));
			}
			else {
				std::string temp = std::to_string(count);
				m_code_run_result->ChangeValue(wxString::FromUTF8(temp.c_str()) + wxT("Load Failed!"));
			}

			now = global;

			dataViewListCtrlNo = -1;
			position = now->get_data_size() > 0? 0 : -1;

			run_count = 0;

			dir_vec = std::vector<std::string>();
			dir_vec.push_back(".");

			dir_text->ChangeValue(wxT(""));

			m_now_view_text->SetLabelText(wxT("View Mode A"));

			part.clear();
			part.push_back(0);

			RefreshText(now);
			RefreshTable(now);

			SetTitle(wxT("SimdclaujsonJson Explorer : ") + _fileName);

			*changed = true;
		}
		openFileDialog->Destroy();
	}
	
	virtual void FileNewMenuOnMenuSelection(wxCommandEvent& event) {
		if (!isMain) { return; }
		
		encoding = Encoding::UTF8;
		//SetConsoleOutputCP(65001); // Windows..
		//setlocale(LC_ALL, "en_US.UTF-8");

		now = nullptr;
		global->remove_all(manager);
		manager.Clear();

		now = global;

		dataViewListCtrlNo = -1;
		
		position = -1;

		run_count = 0;
		
		dir_vec = std::vector<std::string>();
		dir_vec.push_back(".");
		dir_text->ChangeValue(wxT("/."));
		
		m_now_view_text->SetLabelText(wxT("View Mode A"));


		part.clear();
		part.push_back(0);

		RefreshText(now);
		RefreshTable(now);

		m_code_run_result->ChangeValue(wxT("New Success! : UTF-8 encoding."));

		*changed = true;
	}
	
	virtual void FileSaveMenuOnMenuSelection(wxCommandEvent& event) {
		if (!isMain) { return; }
		wxFileDialog* saveFileDialog = new wxFileDialog(this, _("Save"), "", "",
			"", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (saveFileDialog->ShowModal() == wxID_OK)
		{
			string fileName(saveFileDialog->GetPath().c_str());

			claujson::LoadData::save(fileName, *global);

			m_code_run_result->SetLabelText(saveFileDialog->GetPath() + wxT(" is saved.."));
		}
		saveFileDialog->Destroy();
	}
	virtual void FileExitMenuOnMenuSelection(wxCommandEvent& event) { Close(true); }
	virtual void InsertMenuOnMenuSelection(wxCommandEvent& event) {
		if (!isMain) { return; }
		
		if (*changed) { 
			changedEvent();
		}

		//if (-1 == position) {
		//	return;
		//}

		int idx = now->get_data_size();// / 4)* dataViewListCtrlNo;
		
		bool isUserType = false; // now->get_data_list(idx)->is_user_type();

		//if (dataViewListCtrlNo == -1) { return; }

		ChangeWindow* changeWindow = new ChangeWindow(this, &manager, now, isUserType, idx, 2);

		changeWindow->ShowModal();

		if (now) {
			RefreshText(now);
			RefreshTable(now);
		}
	}
	virtual void ChangeMenuOnMenuSelection(wxCommandEvent& event) {
		if (!isMain) { return; }

		if (*changed) { changedEvent();
			return;
		}
		if (-1 == position) { return; }

		int idx = wiz::GetIdx(now, position, (now->get_data_size() / 4) * dataViewListCtrlNo);

		bool isUserType = now->get_data_list(idx)->is_user_type();

		ChangeWindow* changeWindow = new ChangeWindow(this, &manager, now, isUserType,
			idx, 1);

		changeWindow->ShowModal();

		if (now) {
			RefreshText(now);
			RefreshTable(now);
		}
	}
	virtual void RemoveMenuOnMenuSelection(wxCommandEvent& event) {
		if (!isMain) { return; }
		
		if (*changed) { changedEvent();
			return;
		}
		if (-1 == position) { return; }
		
		int idx = wiz::GetIdx(now, position, ((now->get_data_size()) / 4) * dataViewListCtrlNo);
		
		int type = 1;

		bool isUserType = now->get_data_list(idx)->is_user_type();

		if (isUserType) {
			now->remove_data_list(manager, idx);
		}
		else {
			if (now->is_object()) {
				now->remove_data_list(manager, idx + 1);
				now->remove_data_list(manager, idx);
			}
			else {
				now->remove_data_list(manager, idx);
			}
		}
		RefreshText(now);
		RefreshTable(now);
	}
	virtual void back_buttonOnButtonClick(wxCommandEvent& event) {
		if (*changed) {
			changedEvent();
			if (!isMain) { return; }
		}
		if (now && now->get_parent()) {
			part.pop_back();

			RefreshText(now->get_parent());
			RefreshTable(now->get_parent());
			now = now->get_parent();
			BackDir();
		}
	}
	virtual void refresh_buttonOnButtonClick(wxCommandEvent& event) {
		if (*changed) {
			changedEvent();
			if (!isMain) { return; }
		}

		if (now && !part.empty()) {
			std::string dir = "";
			for (int i = 0; i < dir_vec.size(); ++i) {
				dir += "/";
				dir += dir_vec[i];
				dir += part[i] > 0 ?  std::string("part") + std::to_string(part[i]) : "";
			}

			dir_text->ChangeValue(Convert(dir));

			RefreshText(now);
			RefreshTable(now);
		}
	}

	// keyboard
	virtual void m_dataViewListCtrl1OnChar(wxKeyEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}
		dataViewListCtrlNo = 0; position = m_dataViewListCtrl1->GetSelectedRow();

		int size = now->get_data_size() ;

		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		int length = size;


		if (WXK_ESCAPE == event.GetKeyCode()) {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			for (int i = 0; i < 4; ++i) {
				ctrl[i]->UnselectAll();
			}
			position = -1;
		}
		else if (NK_ENTER == event.GetKeyCode() && position >= 0 && now->get_data_list(wiz::GetIdx(now, position, part.back() * part_size))->is_user_type()) { // is_user_type <- position < now->get_data_size()
			now = now->get_data_list(wiz::GetIdx(now, position, part.back() * part_size));
			
			

			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object()? "{" + std::to_string(wiz::GetIdx(now, position , part.back() * part_size))
				 + "}" : "[" + std::to_string(wiz::GetIdx(now, position, part.back() * part_size))  + "]"), part);


			RefreshText(now);
			RefreshTable(now);

		}
		else if (NK_BACKSPACE == event.GetKeyCode() && now->get_parent() != nullptr) {
			now = now->get_parent();
			part.pop_back();
			RefreshText(now);
			RefreshTable(now);
			BackDir();
		}
		else {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			ctrl[dataViewListCtrlNo]->UnselectRow(position);

			if (WXK_UP == event.GetKeyCode() && dataViewListCtrlNo > -1 && position > 0)//< ctrl[dataViewListCtrlNo]->GetItemCount())
			{
			//	RefreshText(now);
				event.Skip();
				return;
			}
			else if (WXK_DOWN == event.GetKeyCode() && dataViewListCtrlNo > -1 && position >= 0 && position < ctrl[dataViewListCtrlNo]->GetItemCount() - 1)
			{
				//RefreshText(now);
				event.Skip();
				return;
			}
			else if (WXK_LEFT == event.GetKeyCode() && dataViewListCtrlNo > 0 && position >= 0 && position < ctrl[dataViewListCtrlNo - 1]->GetItemCount())
			{
				
				dataViewListCtrlNo--; RefreshText2(now);
			}
			else if (WXK_RIGHT == event.GetKeyCode() && dataViewListCtrlNo < 3 && position >= 0 && position < ctrl[dataViewListCtrlNo + 1]->GetItemCount())
			{
				
				dataViewListCtrlNo++; RefreshText2(now);
			}

			ctrl[dataViewListCtrlNo]->SelectRow(position);
			ctrl[dataViewListCtrlNo]->SetFocus();
		}
	}
	virtual void m_dataViewListCtrl2OnChar(wxKeyEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}

		int size = now->get_data_size();

		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		int length = size;

		dataViewListCtrlNo = 1; position = m_dataViewListCtrl2->GetSelectedRow();
		if (WXK_ESCAPE == event.GetKeyCode()) {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			for (int i = 0; i < 4; ++i) {
				ctrl[i]->UnselectAll();
			}
			position = -1;
		}
		else if (NK_ENTER == event.GetKeyCode() && dataViewListCtrlNo == 1 && position >= 0 &&
				now->get_data_list(wiz::GetIdx(now, position, (length) / 4 + part.back() * part_size))->is_user_type()) {
			now = now->get_data_list(wiz::GetIdx(now, position, (length) / 4 + part.back() * part_size));

		
			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object() ? "{" + std::to_string(wiz::GetIdx(now, position, (length) / 4 + part.back() * part_size)
				)  + "}" :
				"[" + std::to_string(wiz::GetIdx(now, position, (length) / 4 + part.back() * part_size) )  + "]"), part);

			RefreshText(now);
			RefreshTable(now);
		}
		else if (NK_BACKSPACE == event.GetKeyCode() && now->get_parent() != nullptr) {
			now = now->get_parent();
			part.pop_back();
			RefreshText(now);
			RefreshTable(now);
			BackDir();
		}
		else {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			ctrl[dataViewListCtrlNo]->UnselectRow(position);

			if (WXK_UP == event.GetKeyCode() && dataViewListCtrlNo > -1 && position > 0)//< ctrl[dataViewListCtrlNo]->GetItemCount())
			{
				//RefreshText(now);
				event.Skip();
				return;
			}
			else if (WXK_DOWN == event.GetKeyCode() && dataViewListCtrlNo > -1 && position >= 0 && position < ctrl[dataViewListCtrlNo]->GetItemCount() - 1)
			{
				//RefreshText(now);
				event.Skip();
				return;
			}
			else if (WXK_LEFT == event.GetKeyCode() && dataViewListCtrlNo > 0 && position >= 0 && position < ctrl[dataViewListCtrlNo - 1]->GetItemCount())
			{
				dataViewListCtrlNo--;
				RefreshText2(now);
			}
			else if (WXK_RIGHT == event.GetKeyCode() && dataViewListCtrlNo < 3 && position >= 0 && position < ctrl[dataViewListCtrlNo + 1]->GetItemCount())
			{
				
				dataViewListCtrlNo++;
				RefreshText2(now);
			}

			ctrl[dataViewListCtrlNo]->SelectRow(position);
			ctrl[dataViewListCtrlNo]->SetFocus();
		}

	}
	virtual void m_dataViewListCtrl3OnChar(wxKeyEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}


		int size = now->get_data_size();

		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		int length = size;

		dataViewListCtrlNo = 2; position = m_dataViewListCtrl3->GetSelectedRow();
		if (WXK_ESCAPE == event.GetKeyCode()) {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			for (int i = 0; i < 4; ++i) {
				ctrl[i]->UnselectAll();
			}
			position = -1;
		}
		else if (NK_ENTER == event.GetKeyCode() && dataViewListCtrlNo == 2 && position >= 0
				&& now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 2 + part.back() * part_size) )->is_user_type()) {
			now = now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 2 + part.back() * part_size) );
			

			
			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object() ? "{" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 2 + part.back() * part_size) 
				)  + "}" :
				"[" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 2 + part.back() * part_size) + (length) / 4 * 2 + part.back() * part_size)  + "]"), part);

			RefreshText(now);
			RefreshTable(now);
		}
		else if (NK_BACKSPACE == event.GetKeyCode() && now->get_parent() != nullptr) {
			now = now->get_parent();
			part.pop_back();

			RefreshText(now);
			RefreshTable(now);
			BackDir();

		}
		else {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			ctrl[dataViewListCtrlNo]->UnselectRow(position);

			if (WXK_UP == event.GetKeyCode() && dataViewListCtrlNo > -1 && position > 0)//< ctrl[dataViewListCtrlNo]->GetItemCount())
			{
				//RefreshText(now);
				event.Skip();
				return;
			}
			else if (WXK_DOWN == event.GetKeyCode() && dataViewListCtrlNo > -1 && position >= 0 && position < ctrl[dataViewListCtrlNo]->GetItemCount() - 1)
			{
				//RefreshText(now);
				event.Skip();
				return;
			}
			else if (WXK_LEFT == event.GetKeyCode() && dataViewListCtrlNo > 0 && position >= 0 && position < ctrl[dataViewListCtrlNo - 1]->GetItemCount())
			{
				
				dataViewListCtrlNo--;
				RefreshText2(now);
			}
			else if (WXK_RIGHT == event.GetKeyCode() && dataViewListCtrlNo < 3 && position >= 0 && position < ctrl[dataViewListCtrlNo + 1]->GetItemCount())
			{
				dataViewListCtrlNo++;
				RefreshText2(now);
			}

			ctrl[dataViewListCtrlNo]->SelectRow(position);
			ctrl[dataViewListCtrlNo]->SetFocus();
		}
	}
	virtual void m_dataViewListCtrl4OnChar(wxKeyEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}

		int size = now->get_data_size();
		

		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		int length = size;
		dataViewListCtrlNo = 3; position = m_dataViewListCtrl4->GetSelectedRow();
		if (WXK_ESCAPE == event.GetKeyCode()) {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			for (int i = 0; i < 4; ++i) {
				ctrl[i]->UnselectAll();
			}
			position = -1;
		}
		else if (NK_ENTER == event.GetKeyCode() && dataViewListCtrlNo == 3 && position >= 0 
				&& now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 3 + part.back() * part_size) )->is_user_type()) {
			now = now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 3 + part.back() * part_size) );

		


			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object() ? "{" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 3 + part.back() * part_size))  + "}" :
				"[" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 3 + part.back() * part_size) + (length) / 4 * 3 + part.back() * part_size)  + "]"), part);

			RefreshText(now);
			RefreshTable(now);
		}
		else if (NK_BACKSPACE == event.GetKeyCode() && now->get_parent() != nullptr) {
			now = now->get_parent();
			part.pop_back();

			RefreshText(now);
			RefreshTable(now);
			BackDir();

		}
		else {
			wxDataViewListCtrl* ctrl[4];
			ctrl[0] = m_dataViewListCtrl1;
			ctrl[1] = m_dataViewListCtrl2;
			ctrl[2] = m_dataViewListCtrl3;
			ctrl[3] = m_dataViewListCtrl4;

			ctrl[dataViewListCtrlNo]->UnselectRow(position);

			if (WXK_UP == event.GetKeyCode() && dataViewListCtrlNo > -1 && position > 0)//< ctrl[dataViewListCtrlNo]->GetItemCount())
			{
				//RefreshText(now);
				event.Skip();
				return;
			}
			else if (WXK_DOWN == event.GetKeyCode() && dataViewListCtrlNo > -1 && position >= 0 && position < ctrl[dataViewListCtrlNo]->GetItemCount() - 1)
			{
				//RefreshText2(now);
				event.Skip();
				return;
			}
			else if (WXK_LEFT == event.GetKeyCode() && dataViewListCtrlNo > 0 && position >= 0 && position < ctrl[dataViewListCtrlNo - 1]->GetItemCount())
			{
				dataViewListCtrlNo--;
				RefreshText2(now);
			}
			else if (WXK_RIGHT == event.GetKeyCode() && dataViewListCtrlNo < 3 && position >= 0 && position < ctrl[dataViewListCtrlNo + 1]->GetItemCount())
			{
				dataViewListCtrlNo++;
				RefreshText2(now);
			}

			ctrl[dataViewListCtrlNo]->SelectRow(position);
			ctrl[dataViewListCtrlNo]->SetFocus();
		}
	}


	// Part - very long? array or object.
	virtual void nextOnButtonClick(wxCommandEvent& event) { 
		if (now) {
			long long length = now->get_data_size();

			if (!part.empty() && length > (part.back() + 1) * part_size) {
				part.back()++;

				RefreshText(now);
				RefreshTable(now);

				std::string dir = "";
				for (int i = 0; i < dir_vec.size(); ++i) {
					dir += "/";
					dir += dir_vec[i];
					dir += part[i] > 0 ?  std::string("part") + std::to_string(part[i]) : "";
				}

				dir_text->ChangeValue(Convert(dir));
			}
		}
	}

	virtual void beforeOnButtonClick(wxCommandEvent& event) {
		if (now) {

			if (!part.empty() && part.back() >= 1) {
				part.back()--;

				RefreshText(now);
				RefreshTable(now);

				std::string dir = "";
				for (int i = 0; i < dir_vec.size(); ++i) {
					dir += "/";
					dir += dir_vec[i];
					dir += part[i] > 0 ? std::string("part") + std::to_string(part[i]) : "";
				}

				dir_text->ChangeValue(Convert(dir));
			}
		}
	}


	// double click.
	virtual void m_dataViewListCtrl1OnDataViewListCtrlItemActivated(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}
		int size = now->get_data_size();

		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		int length = size;
		dataViewListCtrlNo = 0; position = m_dataViewListCtrl1->GetSelectedRow();

		if (position >= 0 && now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 0 + part.back() * part_size) + (length) / 4 * 0 + part.back() * part_size)->is_user_type()) {
			now = now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 0 + part.back() * part_size) + part.back() * part_size);

			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object() ? "{" + 
					std::to_string(wiz::GetIdx(now, position, (length) / 4 * 0 + part.back() * part_size))  + "}" :
				"[" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 0 + part.back() * part_size) )  + "]"), part);


			RefreshText(now);
			RefreshTable(now);

		}
	}
	virtual void m_dataViewListCtrl2OnDataViewListCtrlItemActivated(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}
		int size = now->get_data_size();
		

		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		int length = size;
		dataViewListCtrlNo = 1; position = m_dataViewListCtrl2->GetSelectedRow();
		if (dataViewListCtrlNo == 1 && position >= 0 && now->get_data_list(wiz::GetIdx(now, position , (length) / 4 + part.back() * part_size) )->is_user_type()) {
			now = now->get_data_list(wiz::GetIdx(now, position, (length) / 4 + part.back() * part_size) );
			
			

			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object() ? "{" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 1 + part.back() * part_size) ) + "}" :
				"[" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 1 + part.back() * part_size))  + "]"), part);
			

			RefreshText(now);
			RefreshTable(now);
		}
	}
	virtual void m_dataViewListCtrl3OnDataViewListCtrlItemActivated(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}

		int size = now->get_data_size();
		
		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		int length = size;
		dataViewListCtrlNo = 2; position = m_dataViewListCtrl3->GetSelectedRow();
		if (dataViewListCtrlNo == 2 && position >= 0 && now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 2 + part.back() * part_size))->is_user_type()) {
			now = now->get_data_list(wiz::GetIdx(now, position,  (length) / 4 * 2 + part.back() * part_size));	

			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object() ? "{" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 2 + part.back() * part_size))  + "}" :
				"[" + std::to_string(wiz::GetIdx(now, position, (length) / 4 * 2 + part.back() * part_size) ) + "]"), part);

			RefreshText(now);
			RefreshTable(now);

		}
	}
	virtual void m_dataViewListCtrl4OnDataViewListCtrlItemActivated(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}

		int size = now->get_data_size();

		if (size > part_size) {
			if (size >= part_size * part.back() && size < part_size * (part.back() + 1)) {
				size = size - part_size * part.back();
			}
			else {
				size = part_size;
			}
		}

		long long length = size;

		dataViewListCtrlNo = 3; position = m_dataViewListCtrl4->GetSelectedRow();
		if (dataViewListCtrlNo == 3 && position >= 0 && now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 3 + part.back() * part_size))->is_user_type()) {
			now = now->get_data_list(wiz::GetIdx(now, position, (length) / 4 * 3 + part.back() * part_size));
			
		

			EnterDir(wiz::ToString(now->get_value()) + (now->get_parent()->is_object() ? "{" + 
				std::to_string(wiz::GetIdx(now, position, (length / 4 * 3) + part.back() * part_size))  + "}" :
				"[" + std::to_string(wiz::GetIdx(now, position, (length / 4 * 3) + part.back() * part_size))  + "]"), part);

			RefreshText(now);
			RefreshTable(now);

		}
	}

	// right click.
	virtual void m_dataViewListCtrl1OnDataViewListCtrlItemContextMenu(wxDataViewEvent& event) {
		if (*changed) {
			changedEvent();
		}
		if (now && now->get_parent()) {
			now = now->get_parent();
			part.pop_back();
			RefreshText(now);
			RefreshTable(now);
			BackDir();
		}
	}
	virtual void m_dataViewListCtrl2OnDataViewListCtrlItemContextMenu(wxDataViewEvent& event) {
		if (*changed) {
			changedEvent();
		}
		if (now && now->get_parent()) {
			now = now->get_parent();
			part.pop_back();
			RefreshText(now);
			RefreshTable(now);
			BackDir();
		}
	}
	virtual void m_dataViewListCtrl3OnDataViewListCtrlItemContextMenu(wxDataViewEvent& event) {
		if (*changed) {
			changedEvent();
		}
		if (now && now->get_parent()) {
			now = now->get_parent();
			part.pop_back();
			RefreshText(now);
			RefreshTable(now);
			BackDir();
		}
	}
	virtual void m_dataViewListCtrl4OnDataViewListCtrlItemContextMenu(wxDataViewEvent& event) {
		if (*changed) {
			changedEvent();
		}
		if (now && now->get_parent()) {
			now = now->get_parent();
			part.pop_back();

			RefreshText(now);
			RefreshTable(now);
			BackDir();
		}
	}


	virtual void m_dataViewListCtrl1OnDataViewListCtrlSelectionchanged(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}
		dataViewListCtrlNo = 0;
		position = m_dataViewListCtrl1->GetSelectedRow();

		RefreshText2(now);
	}
	virtual void m_dataViewListCtrl2OnDataViewListCtrlSelectionchanged(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}
		dataViewListCtrlNo = 1;
		position = m_dataViewListCtrl2->GetSelectedRow();

		RefreshText2(now);
	}
	virtual void m_dataViewListCtrl3OnDataViewListCtrlSelectionchanged(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}
		dataViewListCtrlNo = 2;
		position = m_dataViewListCtrl3->GetSelectedRow();

		RefreshText2(now);
	}
	virtual void m_dataViewListCtrl4OnDataViewListCtrlSelectionchanged(wxDataViewEvent& event) {
		if (*changed) { changedEvent();
			if (!isMain) { return; }
		}
		dataViewListCtrlNo = 3;
		position = m_dataViewListCtrl4->GetSelectedRow();

		RefreshText2(now);
	}

	virtual void m_dataViewListCtrl1OnSize(wxSizeEvent& event) {
		m_dataViewListCtrl1->GetColumn(0)->SetWidth(m_dataViewListCtrl1->GetSize().GetWidth() / 2 * 0.92); // check...
		m_dataViewListCtrl1->GetColumn(1)->SetWidth(m_dataViewListCtrl1->GetSize().GetWidth() / 2 * 0.92);
		event.Skip();
	}
	virtual void m_dataViewListCtrl2OnSize(wxSizeEvent& event) {
		m_dataViewListCtrl2->GetColumn(0)->SetWidth(m_dataViewListCtrl2->GetSize().GetWidth() / 2 * 0.92); // check...
		m_dataViewListCtrl2->GetColumn(1)->SetWidth(m_dataViewListCtrl2->GetSize().GetWidth() / 2 * 0.92);
		event.Skip();
	}
	virtual void m_dataViewListCtrl3OnSize(wxSizeEvent& event) {
		m_dataViewListCtrl3->GetColumn(0)->SetWidth(m_dataViewListCtrl3->GetSize().GetWidth() / 2 * 0.92); // check...
		m_dataViewListCtrl3->GetColumn(1)->SetWidth(m_dataViewListCtrl3->GetSize().GetWidth() / 2 * 0.92);
		event.Skip();
	}
	virtual void m_dataViewListCtrl4OnSize(wxSizeEvent& event) {
		m_dataViewListCtrl4->GetColumn(0)->SetWidth(m_dataViewListCtrl4->GetSize().GetWidth() / 2 * 0.92); // check...
		m_dataViewListCtrl4->GetColumn(1)->SetWidth(m_dataViewListCtrl4->GetSize().GetWidth() / 2 * 0.92);
		event.Skip();
	}

	virtual void OtherWindowMenuOnMenuSelection(wxCommandEvent& event) {
		if (*changed) { changedEvent(); }

		if (!isMain) { return; }
		MainFrame* frame = new MainFrame(this->changed, this->global, this->now,  this);

		frame->dir_vec = this->dir_vec;

		frame->part = this->part;

		std::string dir = "";
		for (int i = 0; i < frame->dir_vec.size(); ++i) {
			dir += "/";
			dir += frame->dir_vec[i];

			dir += frame->part[i] > 0 ?  std::string(" part") + std::to_string(frame->part[i]) : "";
		}

		frame->dir_text->ChangeValue(Convert(dir));

		frame->RefreshTable(frame->now);
		frame->RefreshText(frame->now);

		frame->SetTitle(GetTitle() + wxT(" : other window"));

		frame->Show(true);
	}

	virtual void LangMenuOnMenuSelection(wxCommandEvent& event) {
		if (*changed) { changedEvent(); }

		LangFrame* frame = new LangFrame(&this->now, &dataViewListCtrlNo, &position, m_dataViewListCtrl1, m_dataViewListCtrl2,
			m_dataViewListCtrl3, m_dataViewListCtrl4, this);
		
		frame->Show(true);
	}

public:

	MainFrame(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SimdclaujsonJson Explorer"), const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(1024, 512), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
private:
	MainFrame(wiz::SmartPtr<bool> changed, wiz::SmartPtr<claujson::UserType> global, claujson::UserType* now, wxWindow* parent, wxWindowID id = wxID_ANY, 
		const wxString& title = wxT("SimdclaujsonJson Explorer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(1024, 512), 
		long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
public:
	~MainFrame();
	
	void init(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("SimdclaujsonJson Explorer"), const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(1024, 512), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

	void FirstFrame() {
		isMain = true;

		global = new claujson::UserType();
		now = global;
	}
};
MainFrame::MainFrame(wiz::SmartPtr<bool> changed, wiz::SmartPtr<claujson::UserType> global, claujson::UserType* now, wxWindow* parent, wxWindowID id,
	const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style)
{
	init(parent, id, title, pos, size, style);

	this->changed = changed;
	this->global = global;
	this->now = now;


	part.push_back(0);
}
MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style)
{
	changed = new bool;
	
	*changed = false;


	part.push_back(0);

	init(parent, id, title, pos, size, style);
}

void MainFrame::init(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	menuBar = new wxMenuBar(0);
	FileMenu = new wxMenu();


	wxMenuItem* FileNewMenu;
	FileNewMenu = new wxMenuItem(FileMenu, wxID_ANY, wxString(wxT("New")), wxEmptyString, wxITEM_NORMAL);
	FileMenu->Append(FileNewMenu);


	wxMenuItem* FileLoadMenu;
	FileLoadMenu = new wxMenuItem(FileMenu, wxID_ANY, wxString(wxT("Load")), wxEmptyString, wxITEM_NORMAL);
	FileMenu->Append(FileLoadMenu);

	wxMenuItem* FileSaveMenu;
	FileSaveMenu = new wxMenuItem(FileMenu, wxID_ANY, wxString(wxT("Save")), wxEmptyString, wxITEM_NORMAL);
	FileMenu->Append(FileSaveMenu);

	FileMenu->AppendSeparator();

	wxMenuItem* FileExitMenu;
	FileExitMenu = new wxMenuItem(FileMenu, wxID_ANY, wxString(wxT("Exit")), wxEmptyString, wxITEM_NORMAL);
	FileMenu->Append(FileExitMenu);

	FileMenu->AppendSeparator();

	menuBar->Append(FileMenu, wxT("File"));

	DoMenu = new wxMenu();
	wxMenuItem* InsertMenu;
	InsertMenu = new wxMenuItem(DoMenu, wxID_ANY, wxString(wxT("Insert")), wxEmptyString, wxITEM_NORMAL);
	DoMenu->Append(InsertMenu);

	wxMenuItem* ChangeMenu;
	ChangeMenu = new wxMenuItem(DoMenu, wxID_ANY, wxString(wxT("Change")), wxEmptyString, wxITEM_NORMAL);
	DoMenu->Append(ChangeMenu);

	wxMenuItem* RemoveMenu;
	RemoveMenu = new wxMenuItem(DoMenu, wxID_ANY, wxString(wxT("Remove")), wxEmptyString, wxITEM_NORMAL);
	DoMenu->Append(RemoveMenu);

	menuBar->Append(DoMenu, wxT("Do"));

	//wxMenuItem* CodeViewMenu;
	//CodeViewMenu = new wxMenuItem(ViewMenu, wxID_ANY, wxString(wxT("CodeView")), wxEmptyString, wxITEM_NORMAL);
	//ViewMenu->Append(CodeViewMenu);


	WindowMenu = new wxMenu();
	wxMenuItem* OtherWindowMenu;
	OtherWindowMenu = new wxMenuItem(WindowMenu, wxID_ANY, wxString(wxT("OtherWindow")), wxEmptyString, wxITEM_NORMAL);
	WindowMenu->Append(OtherWindowMenu);


	wxMenuItem* langMenu;
	langMenu = new wxMenuItem(WindowMenu, wxID_ANY, wxString(wxT("Lang")), wxEmptyString, wxITEM_NORMAL);
	WindowMenu->Append(langMenu);



	menuBar->Append(WindowMenu, wxT("Window"));


	this->SetMenuBar(menuBar);

	wxBoxSizer* bSizer;
	bSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	back_button = new wxButton(this, wxID_ANY, wxT("¡ã"), wxDefaultPosition, wxDefaultSize, 0);
	back_button->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
		
	bSizer2->Add(back_button, 1, wxALL | wxEXPAND, 5);
	
	next = new wxButton(this, wxID_ANY, wxT("¢º"), wxDefaultPosition, wxDefaultSize, 0);
	next->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));

	bSizer2->Add(next, 0, wxALL | wxEXPAND, 5);

	before = new wxButton(this, wxID_ANY, wxT("¢¸"), wxDefaultPosition, wxDefaultSize, 0);
	before->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString));
	bSizer2->Add(before, 0, wxALL | wxEXPAND, 5);

	dir_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	dir_text->Enable(false);

	bSizer2->Add(dir_text, 13, wxALL | wxEXPAND, 5);

	refresh_button = new wxButton(this, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer2->Add(refresh_button, 1, wxALL | wxEXPAND, 5);


	bSizer->Add(bSizer2, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer(wxHORIZONTAL);

	m_dataViewListCtrl1 = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_ROW_LINES | wxDV_SINGLE);
	bSizer3->Add(m_dataViewListCtrl1, 1, wxALL | wxEXPAND, 5);

	m_dataViewListCtrl2 = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_ROW_LINES | wxDV_SINGLE);
	bSizer3->Add(m_dataViewListCtrl2, 1, wxALL | wxEXPAND, 5);

	m_dataViewListCtrl3 = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_ROW_LINES | wxDV_SINGLE);
	bSizer3->Add(m_dataViewListCtrl3, 1, wxALL | wxEXPAND, 5);

	m_dataViewListCtrl4 = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_ROW_LINES | wxDV_SINGLE);
	bSizer3->Add(m_dataViewListCtrl4, 1, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxVERTICAL);

	textCtrl = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);

	bSizer6->Add(textCtrl, 9, wxEXPAND | wxALL, 5);

	bSizer3->Add(bSizer6, 2, wxEXPAND, 5);


	bSizer->Add(bSizer3, 12, wxEXPAND, 5);


	m_dataViewListCtrl1->AppendTextColumn(wxT("name"));
	m_dataViewListCtrl1->AppendTextColumn(wxT("value"));

	m_dataViewListCtrl2->AppendTextColumn(wxT("name"));
	m_dataViewListCtrl2->AppendTextColumn(wxT("value"));

	m_dataViewListCtrl3->AppendTextColumn(wxT("name"));
	m_dataViewListCtrl3->AppendTextColumn(wxT("value"));

	m_dataViewListCtrl4->AppendTextColumn(wxT("name"));
	m_dataViewListCtrl4->AppendTextColumn(wxT("value"));

	m_statusBar1 = this->CreateStatusBar(1, wxST_SIZEGRIP, wxID_ANY);

	m_statusBar1->SetLabelText(wxT(""));

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxHORIZONTAL);

	m_code_run_result = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	m_code_run_result->Enable(false);

	m_code_run_result->ChangeValue(wxT("UTF-8 encoding."));

	bSizer5->Add(m_code_run_result, 7, wxALL, 5);

	m_now_view_text = new wxStaticText(this, wxID_ANY, wxT("View Mode A"), wxDefaultPosition, wxDefaultSize, 0);
	m_now_view_text->Wrap(-1);
	bSizer5->Add(m_now_view_text, 1, wxALL, 5);


	bSizer->Add(bSizer5, 0, wxEXPAND, 5);


	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);

	// Connect Events

	next->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::nextOnButtonClick), NULL, this);

	before->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::beforeOnButtonClick), NULL, this);


	this->Connect(FileNewMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileNewMenuOnMenuSelection));
	this->Connect(FileLoadMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileLoadMenuOnMenuSelection));
	this->Connect(FileSaveMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileSaveMenuOnMenuSelection));
	this->Connect(FileExitMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileExitMenuOnMenuSelection));
	this->Connect(InsertMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::InsertMenuOnMenuSelection));
	this->Connect(ChangeMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::ChangeMenuOnMenuSelection));
	this->Connect(RemoveMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::RemoveMenuOnMenuSelection));

	back_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::back_buttonOnButtonClick), NULL, this);

	refresh_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::refresh_buttonOnButtonClick), NULL, this);


	m_dataViewListCtrl1->Connect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl1OnSize), NULL, this);
	m_dataViewListCtrl2->Connect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl2OnSize), NULL, this);
	m_dataViewListCtrl3->Connect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl3OnSize), NULL, this);
	m_dataViewListCtrl4->Connect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl4OnSize), NULL, this);

	m_dataViewListCtrl1->Connect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl1OnChar), NULL, this);
	m_dataViewListCtrl2->Connect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl2OnChar), NULL, this);
	m_dataViewListCtrl3->Connect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl3OnChar), NULL, this);
	m_dataViewListCtrl4->Connect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl4OnChar), NULL, this);

	// double click
	m_dataViewListCtrl1->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl1OnDataViewListCtrlItemActivated), NULL, this);
	m_dataViewListCtrl2->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl2OnDataViewListCtrlItemActivated), NULL, this);
	m_dataViewListCtrl3->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl3OnDataViewListCtrlItemActivated), NULL, this);
	m_dataViewListCtrl4->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl4OnDataViewListCtrlItemActivated), NULL, this);

	// right click
	m_dataViewListCtrl1->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl1OnDataViewListCtrlItemContextMenu), NULL, this);
	m_dataViewListCtrl2->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl2OnDataViewListCtrlItemContextMenu), NULL, this);
	m_dataViewListCtrl3->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl3OnDataViewListCtrlItemContextMenu), NULL, this);
	m_dataViewListCtrl4->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl4OnDataViewListCtrlItemContextMenu), NULL, this);

	m_dataViewListCtrl1->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl1OnDataViewListCtrlSelectionchanged), NULL, this);
	m_dataViewListCtrl2->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl2OnDataViewListCtrlSelectionchanged), NULL, this);
	m_dataViewListCtrl3->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl3OnDataViewListCtrlSelectionchanged), NULL, this);
	m_dataViewListCtrl4->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl4OnDataViewListCtrlSelectionchanged), NULL, this);


	this->Connect(OtherWindowMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OtherWindowMenuOnMenuSelection));
	this->Connect(langMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::LangMenuOnMenuSelection));
	//this->Connect(CodeViewMenu->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::CodeViewMenuOnMenuSelection));
}

MainFrame::~MainFrame()
{
	if (isMain) {
		global.remove();
		changed.remove();
	}
	// Disconnect Events
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileNewMenuOnMenuSelection));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileLoadMenuOnMenuSelection));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileSaveMenuOnMenuSelection));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::FileExitMenuOnMenuSelection));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::InsertMenuOnMenuSelection));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::ChangeMenuOnMenuSelection));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::RemoveMenuOnMenuSelection));

	back_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::back_buttonOnButtonClick), NULL, this);
	refresh_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::refresh_buttonOnButtonClick), NULL, this);


	m_dataViewListCtrl1->Disconnect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl1OnSize), NULL, this);
	m_dataViewListCtrl2->Disconnect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl2OnSize), NULL, this);
	m_dataViewListCtrl3->Disconnect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl3OnSize), NULL, this);
	m_dataViewListCtrl4->Disconnect(wxEVT_SIZE, wxSizeEventHandler(MainFrame::m_dataViewListCtrl4OnSize), NULL, this);


	m_dataViewListCtrl1->Disconnect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl1OnChar), NULL, this);
	m_dataViewListCtrl2->Disconnect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl2OnChar), NULL, this);
	m_dataViewListCtrl3->Disconnect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl3OnChar), NULL, this);
	m_dataViewListCtrl4->Disconnect(wxEVT_CHAR, wxKeyEventHandler(MainFrame::m_dataViewListCtrl4OnChar), NULL, this);

	m_dataViewListCtrl1->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl1OnDataViewListCtrlItemActivated), NULL, this);
	m_dataViewListCtrl2->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl2OnDataViewListCtrlItemActivated), NULL, this);
	m_dataViewListCtrl3->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl3OnDataViewListCtrlItemActivated), NULL, this);
	m_dataViewListCtrl4->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl4OnDataViewListCtrlItemActivated), NULL, this);

	m_dataViewListCtrl1->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl1OnDataViewListCtrlItemContextMenu), NULL, this);
	m_dataViewListCtrl2->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl2OnDataViewListCtrlItemContextMenu), NULL, this);
	m_dataViewListCtrl3->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl3OnDataViewListCtrlItemContextMenu), NULL, this);
	m_dataViewListCtrl4->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl4OnDataViewListCtrlItemContextMenu), NULL, this);


	m_dataViewListCtrl1->Disconnect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl1OnDataViewListCtrlSelectionchanged), NULL, this);
	m_dataViewListCtrl2->Disconnect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl2OnDataViewListCtrlSelectionchanged), NULL, this);
	m_dataViewListCtrl3->Disconnect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl3OnDataViewListCtrlSelectionchanged), NULL, this);
	m_dataViewListCtrl4->Disconnect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(MainFrame::m_dataViewListCtrl4OnDataViewListCtrlSelectionchanged), NULL, this);
	
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::OtherWindowMenuOnMenuSelection));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::LangMenuOnMenuSelection));

	//m_code_run_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::m_code_run_buttonOnButtonClick), NULL, this);
	
	next->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::nextOnButtonClick), NULL, this);
	before->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::beforeOnButtonClick), NULL, this);


	//this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::CodeViewMenuOnMenuSelection));
}

class TestApp : public wxApp {
public:
	virtual bool OnInit() {
		MainFrame* frame = new MainFrame(nullptr);
		frame->FirstFrame();
		frame->Show(true);
		return true;
	}
};

IMPLEMENT_APP(TestApp)


