#pragma once
#include <string>

namespace Bgm {

	/// exe와 같은 폴더 기준 기본 경로 (.\assets\bgm.mp3)
	std::wstring defaultPath();

	/// 파일을 열기만 함(재생은 안 함). 실패시 false.
	bool init(const std::wstring& mp3Path);

	/// 재생 시작. loop=true면 무한 반복.
	bool play(bool loop = true);

	/// 일시정지/재개/정지
	void pause();
	void resume();
	void stop();     // stop + close

	/// 0~1000 (WinMM 범위). 700 정도가 무난.
	bool setVolume(int volume0to1000);

	/// 특정 구간 반복 (밀리초 단위). 성공시 true.
	/// 예: setLoopRangeMs(0, 10000);  // 처음 10초 반복
	bool setLoopRangeMs(unsigned long fromMs, unsigned long toMs);

	/// 루프 구간 해제(파일 전체 반복으로 복구)
	void clearLoopRange();

	/// 마지막 에러 문자열(있으면)
	std::wstring lastError();
	void startBgmTest();

} // namespace Bgm
