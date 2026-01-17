/***************************************************************************
//
//    softProjector - an open source media projection software
//    Copyright (C) 2017  Vladislav Kobzar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//    VirtualDisplayArea - Streaming output display with overlays
//    Optimized for OBS capture with logo and lower third overlays
//
***************************************************************************/

import QtQuick
import QtMultimedia

Rectangle {
    id: virtDispArea
    visible: true
    color: "#000000"
    anchors.fill: parent

    // Transition timing property
    property int tranTime: 500

    // Double-buffering for content transitions
    property int mx: 0
    property int my: 0
    property int mTox: 0
    property int mToy: 0

    // Overlay configuration properties
    property string logoImagePath: ""
    property real logoOpacity: 1.0
    property int logoSize: 120  // width/height for logo

    property string lowerThirdText: ""
    property color lowerThirdBgColor: "#000000"
    property color lowerThirdTextColor: "#FFFFFF"
    property string lowerThirdFont: "Arial"
    property int lowerThirdFontSize: 32
    property real lowerThirdHeight: 80  // height of lower third bar

    // Overlay visibility flags
    property bool logoVisible: false
    property bool lowerThirdVisible: false

    // Video playback signals
    signal positionChanged(int position)
    signal durationChanged(int duration)
    signal playbackStateChanged(int state)
    signal playbackStopped()

    // ========== Media Player for video content ==========
    MediaPlayer {
        id: player
        objectName: "player"
        onSourceChanged: console.debug("Video source changed:", player.source)
        onPositionChanged: {
            virtDispArea.positionChanged(player.position)
        }
        onDurationChanged: {
            virtDispArea.durationChanged(player.duration)
        }
        onPlaybackStateChanged: {
            virtDispArea.playbackStateChanged(player.playbackState)
        }
    }

    // ========== Background/Content Rendering ==========

    // Background video output
    VideoOutput {
        id: vidOut
        objectName: "vidOut"
        anchors.fill: parent
    }

    // Double-buffered background images
    Image {
        id: backImage1
        objectName: "backImage1"
        cache: false
        opacity: 1.0
    }

    Image {
        id: backImage2
        objectName: "backImage2"
        cache: false
        opacity: 0.0
    }

    // Double-buffered text/content images
    Image {
        id: textImage1
        objectName: "textImage1"
        cache: false
        opacity: 1.0
    }

    Image {
        id: textImage2
        objectName: "textImage2"
        cache: false
        opacity: 0.0
    }

    // ========== Logo Overlay Layer ==========
    Rectangle {
        id: logoContainer
        objectName: "logoContainer"
        width: logoSize
        height: logoSize
        color: "transparent"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 20
        anchors.topMargin: 20
        z: 100
        opacity: 0.0

        Image {
            id: logoImage
            objectName: "logoImage"
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            smooth: true
            asynchronous: true
            cache: false
        }

        // Fade-in animation
        NumberAnimation {
            id: logoFadeIn
            target: logoContainer
            property: "opacity"
            to: logoOpacity
            duration: tranTime
            easing.type: Easing.InOutQuad
        }

        // Fade-out animation
        NumberAnimation {
            id: logoFadeOut
            target: logoContainer
            property: "opacity"
            to: 0.0
            duration: tranTime
            easing.type: Easing.InOutQuad
        }
    }

    // ========== Lower Third Overlay Layer ==========
    Rectangle {
        id: lowerThirdContainer
        objectName: "lowerThirdContainer"
        width: parent.width
        height: lowerThirdHeight
        color: lowerThirdBgColor
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 100
        opacity: 0.0
        y: parent.height  // Start below screen

        // Lower third background bar
        Rectangle {
            id: lowerThirdBg
            anchors.fill: parent
            color: lowerThirdBgColor
        }

        // Text content
        Text {
            id: lowerThirdTextItem
            objectName: "lowerThirdText"
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            anchors.topMargin: 15
            anchors.bottomMargin: 15
            text: lowerThirdText
            color: lowerThirdTextColor
            font.family: lowerThirdFont
            font.pixelSize: lowerThirdFontSize
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Truncate
            elide: Text.ElideRight
        }

        // Slide-up animation (show)
        NumberAnimation {
            id: lowerThirdSlideUp
            target: lowerThirdContainer
            property: "y"
            to: parent.height - lowerThirdHeight
            duration: tranTime
            easing.type: Easing.OutQuad
        }

        // Fade-in animation (accompanying slide-up)
        NumberAnimation {
            id: lowerThirdFadeIn
            target: lowerThirdContainer
            property: "opacity"
            to: 1.0
            duration: tranTime * 0.7
            easing.type: Easing.InOutQuad
        }

        // Slide-down animation (hide)
        NumberAnimation {
            id: lowerThirdSlideDown
            target: lowerThirdContainer
            property: "y"
            to: parent.height
            duration: tranTime
            easing.type: Easing.InQuad
        }

        // Fade-out animation (accompanying slide-down)
        NumberAnimation {
            id: lowerThirdFadeOut
            target: lowerThirdContainer
            property: "opacity"
            to: 0.0
            duration: tranTime * 0.7
            easing.type: Easing.InOutQuad
        }
    }

    // ========== Content Transition Animations ==========

    // Fade transitions for text content
    ParallelAnimation {
        id: parFade1to2
        running: false
        NumberAnimation { target: textImage1; property: "opacity"; to: 0.0; duration: tranTime; }
        NumberAnimation { target: textImage2; property: "opacity"; to: 1.0; duration: tranTime; }
    }

    ParallelAnimation {
        id: parFade2to1
        running: false
        NumberAnimation { target: textImage1; property: "opacity"; to: 1.0; duration: tranTime; }
        NumberAnimation { target: textImage2; property: "opacity"; to: 0.0; duration: tranTime; }
    }

    // Fade transitions for background images
    ParallelAnimation {
        id: parBackFade1to2
        running: false
        NumberAnimation { target: backImage1; property: "opacity"; to: 0.0; duration: tranTime; }
        NumberAnimation { target: backImage2; property: "opacity"; to: 1.0; duration: tranTime; }
    }

    ParallelAnimation {
        id: parBackFade2to1
        running: false
        NumberAnimation { target: backImage1; property: "opacity"; to: 1.0; duration: tranTime; }
        NumberAnimation { target: backImage2; property: "opacity"; to: 0.0; duration: tranTime; }
    }

    // Horizontal slide transitions (move right)
    ParallelAnimation {
        id: moveTextX1to2
        running: false
        NumberAnimation { target: textImage1; property: "x"; to: mTox; duration: tranTime; }
        NumberAnimation { target: textImage2; property: "x"; to: mx; duration: tranTime; }
    }

    ParallelAnimation {
        id: moveTextX2to1
        running: false
        NumberAnimation { target: textImage1; property: "x"; to: mx; duration: tranTime; }
        NumberAnimation { target: textImage2; property: "x"; to: mTox; duration: tranTime; }
    }

    ParallelAnimation {
        id: moveBackX1to2
        running: false
        NumberAnimation { target: backImage1; property: "x"; to: mTox; duration: tranTime; }
        NumberAnimation { target: backImage2; property: "x"; to: mx; duration: tranTime; }
    }

    ParallelAnimation {
        id: moveBackX2to1
        running: false
        NumberAnimation { target: backImage1; property: "x"; to: mx; duration: tranTime; }
        NumberAnimation { target: backImage2; property: "x"; to: mTox; duration: tranTime; }
    }

    // Vertical slide transitions (move up/down)
    ParallelAnimation {
        id: moveTextY1to2
        running: false
        NumberAnimation { target: textImage1; property: "y"; to: mToy; duration: tranTime; }
        NumberAnimation { target: textImage2; property: "y"; to: my; duration: tranTime; }
    }

    ParallelAnimation {
        id: moveTextY2to1
        running: false
        NumberAnimation { target: textImage1; property: "y"; to: my; duration: tranTime; }
        NumberAnimation { target: textImage2; property: "y"; to: mToy; duration: tranTime; }
    }

    ParallelAnimation {
        id: moveBackY1to2
        running: false
        NumberAnimation { target: backImage1; property: "y"; to: mToy; duration: tranTime; }
        NumberAnimation { target: backImage2; property: "y"; to: my; duration: tranTime; }
    }

    ParallelAnimation {
        id: moveBackY2to1
        running: false
        NumberAnimation { target: backImage1; property: "y"; to: my; duration: tranTime; }
        NumberAnimation { target: backImage2; property: "y"; to: mToy; duration: tranTime; }
    }

    // ========== Video Playback Functions ==========

    function playVideo() {
        if (player.playbackState === MediaPlayer.StoppedState ||
            player.playbackState === MediaPlayer.PausedState) {
            player.play()
        }
    }

    function stopVideo() {
        if (player.playbackState === MediaPlayer.PlayingState ||
            player.playbackState === MediaPlayer.PausedState) {
            player.stop()
        }
    }

    function pauseVideo() {
        if (player.playbackState === MediaPlayer.PlayingState) {
            player.pause()
        }
    }

    function setVideoVolume(level) {
        player.volume = level
    }

    function setVideoMuted(toMute) {
        player.muted = toMute
    }

    function setVideoPosition(position) {
        player.seek(position)
    }

    // ========== Content Transition Functions ==========

    function stopAllTransitions() {
        parFade1to2.stop()
        parFade2to1.stop()
        parBackFade1to2.stop()
        parBackFade2to1.stop()
        moveTextX1to2.stop()
        moveTextX2to1.stop()
        moveBackX1to2.stop()
        moveBackX2to1.stop()
        moveTextY1to2.stop()
        moveTextY2to1.stop()
        moveBackY1to2.stop()
        moveBackY2to1.stop()
    }

    /**
     * Transition text content from image 1 to image 2
     * @param tranType - Transition type:
     *    1 = Fade (parallel)
     *    2 = Fade (sequential)
     *    3 = MoveRight
     *    4 = MoveLeft
     *    5 = MoveUp
     *    6 = MoveDown
     *    other = Instant cut
     */
    function transitionText1to2(tranType) {
        if (tranType === 1) {
            textImage2.opacity = 0.0
            parFade1to2.start()
        }
        else if (tranType === 3) {
            mTox = mx + parent.width
            textImage1.y = my
            textImage1.x = mx
            textImage2.y = my
            textImage2.x = mx - parent.width
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextX1to2.start()
        }
        else if (tranType === 4) {
            mTox = mx - parent.width
            textImage1.y = my
            textImage1.x = mx
            textImage2.y = my
            textImage2.x = mx + parent.width
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextX1to2.start()
        }
        else if (tranType === 5) {
            mToy = my - parent.height
            textImage1.y = my
            textImage1.x = mx
            textImage2.y = my + parent.height
            textImage2.x = mx
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextY1to2.start()
        }
        else if (tranType === 6) {
            mToy = my + parent.height
            textImage1.y = my
            textImage1.x = mx
            textImage2.y = my - parent.height
            textImage2.x = mx
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextY1to2.start()
        }
        else {
            // Instant cut
            textImage1.opacity = 0.0
            textImage2.opacity = 1.0
            textImage1.x = parent.x
            textImage1.y = parent.y
            textImage2.x = parent.x
            textImage2.y = parent.y
        }
    }

    /**
     * Transition text content from image 2 to image 1
     */
    function transitionText2to1(tranType) {
        if (tranType === 1) {
            textImage1.opacity = 0.0
            parFade2to1.start()
        }
        else if (tranType === 3) {
            mTox = mx + parent.width
            textImage1.y = my
            textImage1.x = mx - parent.width
            textImage2.y = my
            textImage2.x = mx
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextX2to1.start()
        }
        else if (tranType === 4) {
            mTox = mx - parent.width
            textImage1.y = my
            textImage1.x = mx + parent.width
            textImage2.y = my
            textImage2.x = mx
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextX2to1.start()
        }
        else if (tranType === 5) {
            mToy = my - parent.height
            textImage1.y = my + parent.height
            textImage1.x = mx
            textImage2.y = my
            textImage2.x = mx
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextY2to1.start()
        }
        else if (tranType === 6) {
            mToy = my + parent.height
            textImage1.y = my - parent.height
            textImage1.x = mx
            textImage2.y = my
            textImage2.x = mx
            textImage1.opacity = 1.0
            textImage2.opacity = 1.0
            moveTextY2to1.start()
        }
        else {
            // Instant cut
            textImage1.opacity = 1.0
            textImage2.opacity = 0.0
            textImage1.x = parent.x
            textImage1.y = parent.y
            textImage2.x = parent.x
            textImage2.y = parent.y
        }
    }

    /**
     * Transition background content from image 1 to image 2
     */
    function transitionBack1to2(tranType) {
        if (tranType === 1 || tranType === 2) {
            backImage2.opacity = 0.0
            parBackFade1to2.start()
        }
        else if (tranType === 3) {
            mTox = mx + parent.width
            backImage1.y = my
            backImage1.x = mx
            backImage2.y = my
            backImage2.x = mx - parent.width
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackX1to2.start()
        }
        else if (tranType === 4) {
            mTox = mx - parent.width
            backImage1.y = my
            backImage1.x = mx
            backImage2.y = my
            backImage2.x = mx + parent.width
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackX1to2.start()
        }
        else if (tranType === 5) {
            mToy = my - parent.height
            backImage1.y = my
            backImage1.x = mx
            backImage2.y = my + parent.height
            backImage2.x = mx
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackY1to2.start()
        }
        else if (tranType === 6) {
            mToy = my + parent.height
            backImage1.y = my
            backImage1.x = mx
            backImage2.y = my - parent.height
            backImage2.x = mx
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackY1to2.start()
        }
        else {
            // Instant cut
            backImage1.opacity = 0.0
            backImage2.opacity = 1.0
            backImage1.x = parent.x
            backImage1.y = parent.y
            backImage2.x = parent.x
            backImage2.y = parent.y
        }
    }

    /**
     * Transition background content from image 2 to image 1
     */
    function transitionBack2to1(tranType) {
        if (tranType === 1 || tranType === 2) {
            backImage1.opacity = 0.0
            parBackFade2to1.start()
        }
        else if (tranType === 3) {
            mTox = mx + parent.width
            backImage1.y = my
            backImage1.x = mx - parent.width
            backImage2.y = my
            backImage2.x = mx
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackX2to1.start()
        }
        else if (tranType === 4) {
            mTox = mx - parent.width
            backImage1.y = my
            backImage1.x = mx + parent.width
            backImage2.y = my
            backImage2.x = mx
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackX2to1.start()
        }
        else if (tranType === 5) {
            mToy = my - parent.height
            backImage1.y = my + parent.height
            backImage1.x = mx
            backImage2.y = my
            backImage2.x = mx
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackY2to1.start()
        }
        else if (tranType === 6) {
            mToy = my + parent.height
            backImage1.y = my - parent.height
            backImage1.x = mx
            backImage2.y = my
            backImage2.x = mx
            backImage1.opacity = 1.0
            backImage2.opacity = 1.0
            moveBackY2to1.start()
        }
        else {
            // Instant cut
            backImage1.opacity = 1.0
            backImage2.opacity = 0.0
            backImage1.x = parent.x
            backImage1.y = parent.y
            backImage2.x = parent.x
            backImage2.y = parent.y
        }
    }

    // ========== Logo Overlay Functions ==========

    /**
     * Show the logo overlay with fade-in animation
     * @param imagePath - Path to the logo image file
     * @param size - Size of the logo (width/height)
     */
    function showLogo(imagePath, size) {
        if (imagePath) {
            logoImage.source = "file:///" + imagePath
            logoSize = size
            logoFadeOut.stop()
            logoFadeIn.start()
        }
    }

    /**
     * Hide the logo overlay with fade-out animation
     */
    function hideLogo() {
        logoFadeIn.stop()
        logoFadeOut.start()
    }

    /**
     * Set logo properties and show immediately
     */
    function setLogoImmediate(imagePath, size, opacity) {
        if (imagePath) {
            logoImage.source = "file:///" + imagePath
        }
        logoSize = size
        logoContainer.opacity = opacity
    }

    // ========== Lower Third Overlay Functions ==========

    /**
     * Show the lower third overlay with slide-up and fade-in animations
     * @param text - Text content to display
     * @param bgColor - Background color (hex string, e.g., "#FF0000")
     * @param textColor - Text color (hex string)
     * @param fontSize - Font size in pixels
     */
    function showLowerThird(text, bgColor, textColor, fontSize) {
        lowerThirdText = text
        if (bgColor) lowerThirdBgColor = bgColor
        if (textColor) lowerThirdTextColor = textColor
        if (fontSize) lowerThirdFontSize = fontSize

        // Stop any existing animations
        lowerThirdSlideDown.stop()
        lowerThirdFadeOut.stop()

        // Start slide-up and fade-in together
        lowerThirdContainer.y = parent.height
        lowerThirdContainer.opacity = 0.0
        lowerThirdSlideUp.start()
        lowerThirdFadeIn.start()
    }

    /**
     * Hide the lower third overlay with slide-down and fade-out animations
     */
    function hideLowerThird() {
        lowerThirdSlideUp.stop()
        lowerThirdFadeIn.stop()
        lowerThirdSlideDown.start()
        lowerThirdFadeOut.start()
    }

    /**
     * Update lower third text without showing/hiding
     */
    function updateLowerThirdText(text) {
        lowerThirdText = text
    }

    /**
     * Set lower third properties and show immediately (no animation)
     */
    function setLowerThirdImmediate(text, bgColor, textColor, fontSize) {
        lowerThirdText = text
        if (bgColor) lowerThirdBgColor = bgColor
        if (textColor) lowerThirdTextColor = textColor
        if (fontSize) lowerThirdFontSize = fontSize

        lowerThirdSlideUp.stop()
        lowerThirdSlideDown.stop()
        lowerThirdFadeIn.stop()
        lowerThirdFadeOut.stop()

        lowerThirdContainer.y = parent.height - lowerThirdHeight
        lowerThirdContainer.opacity = 1.0
    }

    /**
     * Quickly hide lower third (no animation, instant cut)
     */
    function hideLowerThirdImmediate() {
        lowerThirdSlideUp.stop()
        lowerThirdSlideDown.stop()
        lowerThirdFadeIn.stop()
        lowerThirdFadeOut.stop()

        lowerThirdContainer.y = parent.height
        lowerThirdContainer.opacity = 0.0
    }
}
