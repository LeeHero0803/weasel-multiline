#include <windows.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <boost/archive/text_woarchive.hpp>
#include <WeaselIPC.h>
#include <ResponseParser.h>

static void check(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
// Exact version-0 field order of the upstream candidate archive.
struct LegacyCandidate {
  int currentPage=0, totalPages=0, highlighted=0;
  bool is_last_page=false;
  std::vector<weasel::Text> candies, comments, labels;
  template<class A> void serialize(A& a, unsigned) {
    a & currentPage & totalPages & highlighted & is_last_page;
    a & candies & comments & labels;
  }
};
template<class T> std::wstring archive(T& value) {
  std::wstringstream stream;
  { boost::archive::text_woarchive output(stream); output << value; }
  return stream.str();
}
int main(int argc, char** argv) {
  std::cout << std::unitbuf;
  try {
    if (argc > 1 && std::string(argv[1]) == "--navigation") {
      weasel::Client client;
      check(client.Connect(), "cannot connect to installed server");
      client.StartSession();
      weasel::Context context;
      weasel::Status status;
      weasel::UIStyle style;
      weasel::Config config;
      std::wstring commit;
      weasel::ResponseParser parser(&commit,&context,&status,&config,&style);
      auto read=[&](const char* action) {
        check(client.GetResponseData([&](LPWSTR p,DWORD n){return parser(p,n);}),"missing response");
        std::cout << action << ": count=" << context.cinfo.candies.size()
          << " row=" << context.cinfo.row_page_size << " page=" << context.cinfo.currentPage
          << " highlight=" << context.cinfo.highlighted << " layout=" << style.layout_type
          << " ascii=" << status.ascii_mode << "\n";
      };
      read("start");
      client.ClearComposition();
      for(char c:std::string("shi")){client.ProcessKeyEvent(weasel::KeyEvent(c,0));read("type");}
      client.ProcessKeyEvent(weasel::KeyEvent(0xff54,0)); read("down");
      const bool expanded=context.cinfo.row_page_size==6 && context.cinfo.candies.size()>6;
      check(expanded,"Down did not expand candidates");
      int labels=0;
      for(const auto& label:context.cinfo.labels) if(!label.str.empty())++labels;
      check(labels==6,"active row does not have exactly six selection labels");
      client.ProcessKeyEvent(weasel::KeyEvent(0xff54,ibus::RELEASE_MASK)); read("down release");
      check(context.cinfo.currentPage==1,"key release advanced a row");
      client.ProcessKeyEvent(weasel::KeyEvent(0xff54,0)); read("down again");
      check(context.cinfo.currentPage==2 && context.cinfo.highlighted==12,"second down did not advance one row");
      for(int page=3;page<=4;++page){
        client.ProcessKeyEvent(weasel::KeyEvent(0xff54,0));read("down within viewport");
        check(context.cinfo.currentPage==page && context.cinfo.highlighted==page*6,"selection left its row");
      }
      for(int page=5;page<=6;++page){
        auto before=context.cinfo.candies;
        client.ProcessKeyEvent(weasel::KeyEvent(0xff54,0));read("scroll down one row");
        check(context.cinfo.currentPage==page && context.cinfo.highlighted==24,"bottom selection did not remain at bottom");
        check(context.cinfo.candies.size()==30,"scroll did not retain five rows");
        for(int i=0;i<24;++i)
          check(context.cinfo.candies[i].str==before[i+6].str,"scroll down did not retain previous four rows");
      }
      for(int page=5;page>=2;--page){
        client.ProcessKeyEvent(weasel::KeyEvent(0xff52,0));read("up within viewport");
        check(context.cinfo.currentPage==page && context.cinfo.highlighted==(page-2)*6,"up moved the viewport before reaching its top");
      }
      auto before=context.cinfo.candies;
      client.ProcessKeyEvent(weasel::KeyEvent(0xff52,0));read("scroll up one row");
      check(context.cinfo.currentPage==1 && context.cinfo.highlighted==0,"top selection did not remain at top");
      for(int i=0;i<24;++i)
        check(context.cinfo.candies[i+6].str==before[i].str,"scroll up did not retain previous four rows");
      client.ProcessKeyEvent(weasel::KeyEvent(0xff52,0)); read("up to first");
      check(context.cinfo.currentPage==0 && context.cinfo.row_page_size==0 && context.cinfo.candies.size()==6,"first row did not collapse");
      client.ClearComposition();
      client.EndSession(); client.Disconnect();
      std::cout << "PASS: expansion, one-row scrolling in both directions, four-row overlap, active numeric labels, key release and collapse\n";
      return 0;
    }
    if (argc > 1 && std::string(argv[1]) == "--ping") {
      weasel::Client client;
      check(client.Connect(), "cannot connect to installed server");
      client.StartSession();
      const bool alive=client.Echo();
      client.EndSession();
      client.Disconnect();
      check(alive, "installed server did not echo session");
      std::cout << "PASS: installed server created and echoed an IPC session\n";
      return 0;
    }
    weasel::CandidateInfo candidate;
    candidate.candies.emplace_back(L"one");
    candidate.comments.emplace_back(L"");
    candidate.labels.emplace_back(L"1");
    candidate.row_page_size=6;
    LegacyCandidate legacy;
    legacy.candies=candidate.candies;
    legacy.comments=candidate.comments;
    legacy.labels=candidate.labels;
    check(archive(candidate)==archive(legacy), "legacy archive changed");
    weasel::Context context;
    std::wstring commit;
    weasel::ResponseParser parser(&commit,&context);
    parser.Feed(L"action=ctx");
    parser.Feed(L"ctx.cand="+archive(legacy));
    check(context.cinfo.candies.size()==1 && context.cinfo.row_page_size==0,
          "legacy response failed");
    parser.Feed(L"ctx.row_page_size=6");
    check(context.cinfo.row_page_size==6, "expanded response failed");
    parser.Feed(L"ctx.row_page_size=11");
    check(context.cinfo.row_page_size==0, "invalid row length accepted");
    parser.Feed(L"ctx.row_page_size=6");
    parser.Feed(L"ctx.cand="+archive(candidate));
    check(context.cinfo.row_page_size==0, "collapsed response kept rows");
    std::cout << "PASS: original archive, old/new responses, bounds, collapse\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}
