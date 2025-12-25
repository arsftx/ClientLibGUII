set(SOURCE_FILES
        src/SRIFLib/NInterfaceResource.cpp src/SRIFLib/NInterfaceResource.h
        src/SRIFLib/GNFontChar.h
        src/SRIFLib/NGFontTexture.cpp src/SRIFLib/NGFontTexture.h
        src/SRIFLib/NIFTextBoard.cpp src/SRIFLib/NIFTextBoard.h
        src/SRIFLib/NIFWnd.cpp src/SRIFLib/NIFWnd.h
        src/SRIFLib/NIFUnderMenuBar.cpp src/SRIFLib/NIFUnderMenuBar.h
        src/SRIFLib/NIFCommunityWnd.cpp src/SRIFLib/NIFCommunityWnd.h
        src/SRIFLib/NIFBlockWnd.cpp src/SRIFLib/NIFBlockWnd.h
        src/SRIFLib/NIFTileWnd.cpp src/SRIFLib/NIFTileWnd.h
        src/SRIFLib/NIFFrame.cpp src/SRIFLib/NIFFrame.h
        src/SRIFLib/NIFMainFrame.cpp src/SRIFLib/NIFMainFrame.h
        src/SRIFLib/NIFButton.h
        src/SRIFLib/NIFDragableArea.h
        src/SRIFLib/IFSlotWithHelpEx.h)

add_library(SROInterfaceLIB STATIC ${SOURCE_FILES})
target_include_directories(SROInterfaceLIB PUBLIC src/)
target_link_libraries(SROInterfaceLIB
        BSLib
        # at the end need to add its, shit!
        ClientLib
        GFXFileManagerLIB
        GFX3DFunction
        abi-testing
        d3d9
        )
