(() => {
  const backgroundLayers = [document.getElementById('backgroundA'), document.getElementById('backgroundB')];
  const textLayers = [document.getElementById('textA'), document.getElementById('textB')];
  const backgroundVideo = document.getElementById('backgroundVideo');
  const mainVideo = document.getElementById('mainVideo');
  const overlay = document.getElementById('overlay');
  const offline = document.getElementById('offline');
  const stage = document.getElementById('stage');

  let backgroundIndex = 0;
  let textIndex = 0;

  function applyFade(element, fade) {
    element.classList.toggle('fade', fade);
  }

  function swapLayer(layers, indexRef, url, enabled, fade) {
    const nextIndex = indexRef.value === 0 ? 1 : 0;
    const active = layers[nextIndex];
    const inactive = layers[indexRef.value];

    applyFade(active, fade);
    applyFade(inactive, fade);

    if (!enabled) {
      inactive.classList.remove('active');
      active.classList.remove('active');
      indexRef.value = nextIndex;
      return;
    }

    if (active.dataset.src !== url) {
      active.dataset.src = url;
      active.src = url;
    }

    active.classList.add('active');
    inactive.classList.remove('active');
    indexRef.value = nextIndex;
  }

  function setVideoFillMode(video, fillMode, isMain) {
    if (isMain) {
      video.style.objectFit = 'contain';
      return;
    }

    if (fillMode === 1) {
      video.style.objectFit = 'contain';
    } else if (fillMode === 2) {
      video.style.objectFit = 'cover';
    } else {
      video.style.objectFit = 'fill';
    }
  }

  async function syncVideo(video, state, options = {}) {
    const { muted = false, loop = false, isMain = false } = options;
    video.loop = loop;
    video.muted = muted;

    if (!state.active) {
      video.pause();
      video.removeAttribute('src');
      video.load();
      video.classList.remove('active');
      return;
    }

    setVideoFillMode(video, state.fillMode || 0, isMain);
    if (video.dataset.src !== state.url) {
      video.dataset.src = state.url;
      video.src = state.url;
      video.load();
    }

    if (typeof state.volume === 'number') {
      video.volume = Math.max(0, Math.min(1, state.volume / 100));
    }
    if (typeof state.muted === 'boolean') {
      video.muted = state.muted;
    }

    if (typeof state.positionMs === 'string') {
      const seconds = Number(state.positionMs) / 1000;
      if (!Number.isNaN(seconds) && Math.abs(video.currentTime - seconds) > 0.4) {
        try {
          video.currentTime = seconds;
        } catch (err) {
        }
      }
    }

    video.classList.add('active');
    if (state.paused) {
      video.pause();
      return;
    }

    try {
      await video.play();
    } catch (err) {
    }
  }

  function applyState(state) {
    offline.style.display = 'none';
    stage.style.width = `${state.width}px`;
    stage.style.height = `${state.height}px`;

    const fade = state.transition === 'fade';
    swapLayer(backgroundLayers, { get value() { return backgroundIndex; }, set value(v) { backgroundIndex = v; } }, state.backgroundImage.url, state.backgroundImage.enabled, fade);
    swapLayer(textLayers, { get value() { return textIndex; }, set value(v) { textIndex = v; } }, state.textImage.url, state.textImage.enabled, fade);

    if (state.overlay.enabled) {
      if (overlay.dataset.src !== state.overlay.url) {
        overlay.dataset.src = state.overlay.url;
        overlay.src = state.overlay.url;
      }
      overlay.classList.add('active');
    } else {
      overlay.classList.remove('active');
    }

    syncVideo(backgroundVideo, state.backgroundVideo, { muted: true, loop: state.backgroundVideo.loop, isMain: false });
    syncVideo(mainVideo, state.mainVideo, { muted: state.mainVideo.muted, loop: false, isMain: true });
  }

  function connect() {
    const socket = new WebSocket(`ws://${window.location.hostname}:15172`);
    socket.addEventListener('message', (event) => {
      const state = JSON.parse(event.data);
      if (state.type === 'state') {
        applyState(state);
      }
    });

    socket.addEventListener('close', () => {
      offline.style.display = 'flex';
      setTimeout(connect, 1000);
    });

    socket.addEventListener('error', () => {
      socket.close();
    });
  }

  connect();
})();
