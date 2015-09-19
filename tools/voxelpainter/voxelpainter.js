var BLOCK_SIZE = 1;
var GRID_SIZE = 30;
var container;
var camera, controls, scene, renderer;
var globalPlane, plane, cube, voxelParent, lastPlaced;
var mouse, raycaster, isShiftDown = false;
var cursorMesh;

var cubeCursorMesh, planeCursorMesh;
var cubeGeo, cubeMaterial;
var planeGeo, planeMaterial;

var objects = [];

init();
render();

function download(filename, text) {
	var element = document.createElement('a');
	element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
	element.setAttribute('download', filename);
	element.style.display = 'none';
	document.body.appendChild(element);
	element.click();
	document.body.removeChild(element);
}

function exportToObj(scene) {
	var exporter = new THREE.OBJExporter();
	var result = exporter.parse(scene);
	download("object.obj", result);
}

function init() {
	container = document.getElementById('container');
	document.getElementById('export-obj').addEventListener('click', function() {
		exportToObj(voxelParent);
	}, false);

	camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 1000);
	var camPosRef = BLOCK_SIZE * GRID_SIZE * 0.05;
	camera.position.set(camPosRef * 5, camPosRef * 8, camPosRef * 13);
	camera.lookAt(new THREE.Vector3());

	scene = new THREE.Scene();
	voxelParent = new THREE.Object3D();
	scene.add(voxelParent);

	// roll-over helpers
	var cursorMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000, opacity: 0.5, transparent: true });
	var cubeCursorGeo = new THREE.BoxGeometry(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
	var planeCursorGeo = new THREE.PlaneBufferGeometry(BLOCK_SIZE, BLOCK_SIZE);
	planeCursorGeo.rotateX(-Math.PI / 2);
	planeCursorGeo.computeBoundingBox();
	cubeCursorGeo.computeBoundingBox();
	cubeCursorMesh = new THREE.Mesh(cubeCursorGeo, cursorMaterial);
	planeCursorMesh = new THREE.Mesh(planeCursorGeo, cursorMaterial);
	scene.add(cubeCursorMesh);
	scene.add(planeCursorMesh);
	cursorMesh = cubeCursorMesh;
	planeCursorMesh.visible = false;

	// primitives

	cubeGeo = new THREE.BoxGeometry(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
	cubeMaterial = new THREE.MeshLambertMaterial({ color: 0xfeb74c });

	planeGeo = new THREE.PlaneBufferGeometry(BLOCK_SIZE, BLOCK_SIZE);
	planeGeo.rotateX(-Math.PI / 2);
	planeMaterial = new THREE.MeshLambertMaterial({ color: 0xfe944c });

	// grid

	var size = BLOCK_SIZE * GRID_SIZE / 2, step = BLOCK_SIZE;
	var geometry = new THREE.Geometry();

	for (var i = - size; i <= size; i += step) {
		geometry.vertices.push(new THREE.Vector3(-size, 0, i));
		geometry.vertices.push(new THREE.Vector3( size, 0, i));

		geometry.vertices.push(new THREE.Vector3(i, 0, -size));
		geometry.vertices.push(new THREE.Vector3(i, 0,  size));
	}

	var material = new THREE.LineBasicMaterial({ color: 0x000000, opacity: 0.2, transparent: true });
	var line = new THREE.LineSegments(geometry, material);
	scene.add(line);

	//

	raycaster = new THREE.Raycaster();
	mouse = new THREE.Vector2();

	var geometry = new THREE.PlaneBufferGeometry(BLOCK_SIZE * GRID_SIZE, BLOCK_SIZE * GRID_SIZE);
	geometry.rotateX(-Math.PI / 2);

	globalPlane = new THREE.Mesh(geometry, new THREE.MeshBasicMaterial({ visible: false }));
	scene.add(globalPlane);
	objects.push(globalPlane);

	// Lights

	var ambientLight = new THREE.AmbientLight(0x606060);
	scene.add(ambientLight);

	var directionalLight = new THREE.DirectionalLight(0xffffff);
	directionalLight.position.set(1, 0.75, 0.5).normalize();
	scene.add(directionalLight);

	renderer = new THREE.WebGLRenderer({ antialias: true });
	renderer.setClearColor(0xf0f0f0);
	renderer.setPixelRatio(window.devicePixelRatio);
	renderer.setSize(window.innerWidth, window.innerHeight);
	container.appendChild(renderer.domElement);
	renderer.domElement.addEventListener('contextmenu', function(e) { e.preventDefault(); }, false);

	controls = new THREE.OrbitControls(camera, renderer.domElement);
	controls.mouseButtons.ORBIT = THREE.MOUSE.RIGHT;
	controls.mouseButtons.PAN = null;

	document.addEventListener('mousemove', onDocumentMouseMove, false);
	document.addEventListener('mousedown', onDocumentMouseDown, false);
	document.addEventListener('mouseup', onDocumentMouseUp, false);
	document.addEventListener('keydown', onDocumentKeyDown, false);
	document.addEventListener('keyup', onDocumentKeyUp, false);

	//

	window.addEventListener('resize', onWindowResize, false);
}

function onWindowResize() {
	camera.aspect = window.innerWidth / window.innerHeight;
	camera.updateProjectionMatrix();
	renderer.setSize(window.innerWidth, window.innerHeight);
}

function onDocumentMouseMove(event) {
	event.preventDefault();
	mouse.set((event.clientX / window.innerWidth) * 2 - 1, - (event.clientY / window.innerHeight) * 2 + 1);
	raycaster.setFromCamera(mouse, camera);
	var intersects = raycaster.intersectObjects(objects);
	if (intersects.length > 0) {
		var intersect = intersects[0];
		cursorMesh.position.copy(intersect.point).add(intersect.face.normal.clone().multiplyScalar(0.5));
		cursorMesh.position.divideScalar(BLOCK_SIZE).floor().multiplyScalar(BLOCK_SIZE).add(cursorMesh.geometry.boundingBox.size().clone().multiplyScalar(0.5));
	}
	if (event.buttons == 1)
		onDocumentMouseDown(event);
}

function onDocumentMouseDown(event) {
	event.preventDefault();
	mouse.set((event.clientX / window.innerWidth) * 2 - 1, - (event.clientY / window.innerHeight) * 2 + 1);
	raycaster.setFromCamera(mouse, camera);
	var intersects = raycaster.intersectObjects(objects);
	if (intersects.length > 0) {
		var intersect = intersects[0];
		// delete object
		if (event.button == 2 || isShiftDown) {
			if (intersect.object != globalPlane) {
				voxelParent.remove(intersect.object);
				objects.splice(objects.indexOf(intersect.object), 1);
			}
		// create cube
		} else if (event.button == 0 && cursorMesh === cubeCursorMesh) {
			var voxel = new THREE.Mesh(cubeGeo, cubeMaterial);
			voxel.position.copy(intersect.point).add(intersect.face.normal.clone().multiplyScalar(0.5));
			voxel.position.divideScalar(BLOCK_SIZE).floor().multiplyScalar(BLOCK_SIZE).add(cursorMesh.geometry.boundingBox.size().clone().multiplyScalar(0.5));
			if (!lastPlaced || Math.abs(lastPlaced.position.y - voxel.position.y) < 0.001) {
				voxelParent.add(voxel);
				objects.push(voxel);
				lastPlaced = voxel;
			}
		// create plane, only on floor level
		} else if (event.button == 0 && cursorMesh === planeCursorMesh && intersect.object === globalPlane) {
			var voxel = new THREE.Mesh(planeGeo, planeMaterial);
			voxel.position.copy(intersect.point).add(intersect.face.normal.clone().multiplyScalar(0.5));
			voxel.position.divideScalar(BLOCK_SIZE).floor().multiplyScalar(BLOCK_SIZE).add(cursorMesh.geometry.boundingBox.size().clone().multiplyScalar(0.5));
			if (!lastPlaced || Math.abs(voxel.position.y) < 0.001) {
				voxelParent.add(voxel);
				objects.push(voxel);
				lastPlaced = voxel;
			}
		}
	}
}

function onDocumentMouseUp(event) {
	event.preventDefault();
	lastPlaced = null;
}

function onDocumentKeyDown(event) {
	switch(event.keyCode) {
		case 16: isShiftDown = true; break;
	}
}

function onDocumentKeyUp(event) {
	switch (event.keyCode) {
		case 16: isShiftDown = false; break;
		case 32:
			cursorMesh.visible = false;
			cursorMesh = cursorMesh == cubeCursorMesh ? planeCursorMesh : cubeCursorMesh;
			cursorMesh.visible = true;
			break;
	}
}

function render() {
	requestAnimationFrame(render);
	controls.update();
	renderer.render(scene, camera);
}
