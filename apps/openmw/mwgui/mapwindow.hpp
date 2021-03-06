#ifndef MWGUI_MAPWINDOW_H
#define MWGUI_MAPWINDOW_H

#include <stdint.h>

#include "windowpinnablebase.hpp"

#include <components/esm/cellid.hpp>

#include <components/esm/custommarkerstate.hpp>

namespace MWRender
{
    class GlobalMap;
}

namespace ESM
{
    class ESMReader;
    class ESMWriter;
}

namespace Loading
{
    class Listener;
}

namespace MWGui
{

    class CustomMarkerCollection
    {
    public:
        void addMarker(const ESM::CustomMarker& marker, bool triggerEvent=true);
        void deleteMarker (const ESM::CustomMarker& marker);
        void updateMarker(const ESM::CustomMarker& marker, const std::string& newNote);

        void clear();

        size_t size() const;

        std::vector<ESM::CustomMarker>::const_iterator begin() const;
        std::vector<ESM::CustomMarker>::const_iterator end() const;

        typedef MyGUI::delegates::CMultiDelegate0 EventHandle_Void;
        EventHandle_Void eventMarkersChanged;

    private:
        std::vector<ESM::CustomMarker> mMarkers;
    };

    class LocalMapBase
    {
    public:
        LocalMapBase(CustomMarkerCollection& markers);
        virtual ~LocalMapBase();
        void init(MyGUI::ScrollView* widget, MyGUI::ImageBox* compass, int mapWidgetSize);

        void setCellPrefix(const std::string& prefix);
        void setActiveCell(const int x, const int y, bool interior=false);
        void setPlayerDir(const float x, const float y);
        void setPlayerPos(int cellX, int cellY, const float nx, const float ny);

        void onFrame(float dt);

        bool toggleFogOfWar();

        struct MarkerUserData
        {
            bool interior;
            int cellX;
            int cellY;
            float nX;
            float nY;
            std::vector<std::string> notes;
            std::string caption;
        };

    protected:
        int mCurX, mCurY;
        bool mInterior;
        MyGUI::ScrollView* mLocalMap;
        MyGUI::ImageBox* mCompass;
        std::string mPrefix;
        bool mChanged;
        bool mFogOfWar;

        int mMapWidgetSize;

        // Stores markers that were placed by a player. May be shared between multiple map views.
        CustomMarkerCollection& mCustomMarkers;

        std::vector<MyGUI::ImageBox*> mMapWidgets;
        std::vector<MyGUI::ImageBox*> mFogWidgets;

        // Keep track of created marker widgets, just to easily remove them later.
        std::vector<MyGUI::Widget*> mDoorMarkerWidgets;
        std::vector<MyGUI::Widget*> mMagicMarkerWidgets;
        std::vector<MyGUI::Widget*> mCustomMarkerWidgets;

        void updateCustomMarkers();

        void applyFogOfWar();

        MyGUI::IntPoint getMarkerPosition (float worldX, float worldY, MarkerUserData& markerPos);

        virtual void notifyPlayerUpdate() {}
        virtual void notifyMapChanged() {}

        virtual void customMarkerCreated(MyGUI::Widget* marker) {}
        virtual void doorMarkerCreated(MyGUI::Widget* marker) {}

        void updateMagicMarkers();
        void addDetectionMarkers(int type);

        void redraw();

        float mMarkerUpdateTimer;

        float mLastDirectionX;
        float mLastDirectionY;
    };

    class EditNoteDialog : public MWGui::WindowModal
    {
    public:
        EditNoteDialog();

        virtual void open();
        virtual void exit();

        void showDeleteButton(bool show);
        bool getDeleteButtonShown();
        void setText(const std::string& text);
        std::string getText();

        typedef MyGUI::delegates::CMultiDelegate0 EventHandle_Void;

        EventHandle_Void eventDeleteClicked;
        EventHandle_Void eventOkClicked;

    private:
        void onCancelButtonClicked(MyGUI::Widget* sender);
        void onOkButtonClicked(MyGUI::Widget* sender);
        void onDeleteButtonClicked(MyGUI::Widget* sender);

        MyGUI::TextBox* mTextEdit;
        MyGUI::Button* mOkButton;
        MyGUI::Button* mCancelButton;
        MyGUI::Button* mDeleteButton;
    };

    class MapWindow : public MWGui::WindowPinnableBase, public LocalMapBase, public NoDrop
    {
    public:
        MapWindow(CustomMarkerCollection& customMarkers, DragAndDrop* drag, const std::string& cacheDir);
        virtual ~MapWindow();

        void setCellName(const std::string& cellName);

        virtual void setAlpha(float alpha);

        void renderGlobalMap(Loading::Listener* loadingListener);

        // adds the marker to the global map
        void addVisitedLocation(const std::string& name, int x, int y);

        // reveals this cell's map on the global map
        void cellExplored(int x, int y);

        void setGlobalMapPlayerPosition (float worldX, float worldY);
        void setGlobalMapPlayerDir(const float x, const float y);

        virtual void open();

        void onFrame(float dt);

        /// Clear all savegame-specific data
        void clear();

        void write (ESM::ESMWriter& writer, Loading::Listener& progress);
        void readRecord (ESM::ESMReader& reader, uint32_t type);

    private:
        void onDragStart(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
        void onMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
        void onWorldButtonClicked(MyGUI::Widget* _sender);
        void onMapDoubleClicked(MyGUI::Widget* sender);
        void onCustomMarkerDoubleClicked(MyGUI::Widget* sender);
        void onNoteEditOk();
        void onNoteEditDelete();
        void onNoteEditDeleteConfirm();
        void onNoteDoubleClicked(MyGUI::Widget* sender);
        void onChangeScrollWindowCoord(MyGUI::Widget* sender);
        void globalMapUpdatePlayer();

        MyGUI::ScrollView* mGlobalMap;
        MyGUI::ImageBox* mGlobalMapImage;
        MyGUI::ImageBox* mGlobalMapOverlay;
        MyGUI::ImageBox* mPlayerArrowLocal;
        MyGUI::ImageBox* mPlayerArrowGlobal;
        MyGUI::Button* mButton;
        MyGUI::IntPoint mLastDragPos;
        bool mGlobal;

        MyGUI::IntCoord mLastScrollWindowCoordinates;

        // Markers on global map
        typedef std::pair<int, int> CellId;
        std::set<CellId> mMarkers;

        // Cells that should be explored in the next frame (i.e. their map revealed on the global map)
        // We can't do this immediately, because the map update is not immediate either (see mNeedMapUpdate in scene.cpp)
        std::vector<CellId> mQueuedToExplore;

        MyGUI::Button* mEventBoxGlobal;
        MyGUI::Button* mEventBoxLocal;

        MWRender::GlobalMap* mGlobalMapRender;

        EditNoteDialog mEditNoteDialog;
        ESM::CustomMarker mEditingMarker;

        virtual void onPinToggled();
        virtual void onTitleDoubleClicked();

        virtual void doorMarkerCreated(MyGUI::Widget* marker);
        virtual void customMarkerCreated(MyGUI::Widget *marker);

        virtual void notifyPlayerUpdate();

    };
}
#endif
