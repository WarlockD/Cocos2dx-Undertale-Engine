#include "Face.h"
#include "UndertaleResources.h"

namespace Undertale {

	Face::Face() : _faceEmotion(0) {}

	bool Face::init(UndertaleFaces face, int faceEmotion)
	{
		return false;
	}

	void Face::blink()
	{
	}

	Face * Face::create(UndertaleFaces face, int faceEmotion)
	{
		Face* f = new Face();
		if (f && f->init(face, faceEmotion)) {
			f->autorelease();
			return f;
		}
		return nullptr;
	}

	void Face::setFaceEmotion(int value)
	{
	}
	void Face::setFace(int faceEmotion, UndertaleFaces face) {
		if (faceEmotion == _faceEmotion && face == _currentFace) return;
		switch (face) {
		case UndertaleFaces::Papyrus:
			switch (faceEmotion) {
			case 0: setSpriteName("spr_face_papyrus"); break;
			case 1: setSpriteName("spr_face_papyrusmad"); break;
			case 2: setSpriteName("spr_face_papyruslaugh"); break;
			case 3: setSpriteName("spr_face_papyrusside"); break;
			case 4: setSpriteName("spr_face_papyrusevil"); break;
			case 5: setSpriteName("spr_face_papyrusside"); break;
			case 6: setSpriteName("spr_face_papyrusdejected"); break;
			case 7: setSpriteName("spr_face_papyruswacky"); break;
			case 8: setSpriteName("spr_face_papyruscry"); break;
			case 9: setSpriteName("spr_face_papyruscool"); break;
			default:
				CCLOGERROR("Bad FaceEmotion setting %i", _faceEmotion);
				return;
			}
			break;
		case UndertaleFaces::TorielTalk:
			switch (faceEmotion) {
			case 0: setSpriteName("spr_face_torielhappytalk"); break;
			case 1: setSpriteName("spr_face_torieltalkside"); break;
			case 2: setSpriteName("spr_face_torieltalk"); break;
			case 3: setSpriteName("spr_face_torielwhat"); break;
			case 4: setSpriteName("spr_face_torielwhatside"); break;
			case 5: setSpriteName("spr_face_torielrevenge"); break;
			case 6: setSpriteName("spr_face_torielcold"); break;
			case 7: setSpriteName("spr_face_torielmad"); break;
			case 8: setSpriteName("spr_face_torielembarrassed"); break;
			case 9: setSpriteName("spr_face_toriel_goawayasgore"); break;
			default:
				CCLOGERROR("Bad FaceEmotion setting %i", _faceEmotion);
				return;
			}
			break;

		}
	}
	void Face::setFace(UndertaleFaces face)
	{
		if (_currentFace == face) return;
		_currentFace = face;
		UndertaleResources* res = UndertaleResources::getInstance();
		switch (face) {
		case UndertaleFaces::Papyrus:
			_faceEmotionFrames[0] = res->getSpriteFrames("spr_face_papyrus");
			_faceEmotionFrames[1] = res->getSpriteFrames("spr_face_papyrusmad");
			_faceEmotionFrames[2] = res->getSpriteFrames("spr_face_papyruslaugh");
			_faceEmotionFrames[3] = res->getSpriteFrames("spr_face_papyrusside");
			_faceEmotionFrames[4] = res->getSpriteFrames("spr_face_papyrusevil");
			_faceEmotionFrames[5] = res->getSpriteFrames("spr_face_papyrusside");
			_faceEmotionFrames[6] = res->getSpriteFrames("spr_face_papyrusdejected");
			_faceEmotionFrames[7] = res->getSpriteFrames("spr_face_papyruswacky");
			_faceEmotionFrames[8] = res->getSpriteFrames("spr_face_papyruscry");
			_faceEmotionFrames[9] = res->getSpriteFrames("spr_face_papyruscool");
			break;
		}
	}








}