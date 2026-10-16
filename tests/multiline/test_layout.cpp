#include "stdafx.h"
#include <iostream>
#include <stdexcept>
#include "HorizontalLayout.h"
using namespace weasel;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
  try {
    Gdiplus::GdiplusStartupInput gdip;
    ULONG_PTR token;
    check(Gdiplus::GdiplusStartup(&token,&gdip,nullptr)==Gdiplus::Ok,"GDI+ startup");
    UIStyle style;
    style.layout_type=UIStyle::LAYOUT_HORIZONTAL;
    style.font_face=style.label_font_face=style.comment_font_face=L"PingFang SC Semibold";
    style.font_point=style.label_font_point=14;
    style.comment_font_point=13;
    style.inline_preedit=true; style.client_caps=INLINE_PREEDIT_CAPABLE;
    style.margin_x=style.margin_y=8;
    style.candidate_spacing=10; style.hilite_spacing=4;
    style.hilite_padding_x=style.hilite_padding_y=4;
    style.hilited_mark_color=0xffffffff;
    style.hilited_comment_text_color=0xffffffff;
    style.comment_text_color=0; // Selection-dependent visibility must not resize.
    auto dwr=std::make_shared<DirectWriteResources>(style,96);
    Status status; status.composing=true;
    Context context; context.cinfo.row_page_size=6;
    for(int i=0;i<29;++i){
      context.cinfo.candies.emplace_back(i%7==0?L"词组较长":L"词");
      context.cinfo.comments.emplace_back(i%5==0?L"comment":L"");
      context.cinfo.labels.emplace_back(L"");
    }
    HDC screen=GetDC(nullptr);
    CDCHandle dc(screen);
    CRect baseline[29]; CSize baselineSize;
    for(int row=0;row<5;++row){
      for(auto& label:context.cinfo.labels)label.str.clear();
      context.cinfo.highlighted=row*6;
      for(int i=row*6;i<(row+1)*6 && i<29;++i)
        context.cinfo.labels[i].str=std::to_wstring(i%6+1);
      HorizontalLayout layout(style,context,status,dwr);
      layout.DoLayout(dc,dwr);
      for(int i=0;i<29;++i){
        const CRect rect=layout.GetCandidateTextRect(i);
        check(rect.left==layout.GetCandidateTextRect(i%6).left,"columns do not align");
        check(layout.GetCandidateLabelRect(i).Width()>0,"preview label space missing");
        if(!row)baseline[i]=rect;
        else check(rect==baseline[i],"row change moved a candidate");
      }
      if(!row)baselineSize=layout.GetContentSize();
      else check(layout.GetContentSize()==baselineSize,"row change resized panel");
      const auto highlight=layout.GetHighlightRect();
      check(highlight.left==layout.GetCandidateRect(row*6).left,"highlight misses row start");
      check(highlight.right==layout.GetCandidateRect((std::min)(row*6+5,28)).right,"highlight misses row end");
      for(int i=row*6;i<(row+1)*6 && i<29;++i)
        check(highlight.PtInRect(layout.GetCandidateTextRect(i).CenterPoint()),"highlight misses active word");
    }
    ReleaseDC(nullptr,screen);
    Gdiplus::GdiplusShutdown(token);
    std::cout << "PASS: fixed label slots and columns; no movement/resize across 5 rows; full-row highlight; partial last row; hidden comments; mark gutter\n";
    return 0;
  }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
}
