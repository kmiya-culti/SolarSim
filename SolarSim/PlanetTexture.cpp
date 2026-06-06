
#include "SolarSim.h"

#ifdef	DIRECT3D_SWAP

#ifdef EARTH_MONTH_IMAGE
HRESULT CreateEarthAnimeTexture() {

	#include "image/earth_anime.h"

	HRESULT hr = SEVERITY_SUCCESS;
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 256;
	desc.Height = 128;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc = { 1, 0 };
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.SysMemPitch = 4 * 256;

	for (int n = 0; n < 12; n++) {
		initData.pSysMem = earth_img[n];

		hr = pD3DDevice->CreateTexture2D(&desc, &initData, &pEarthTexture[n]);
		if (FAILED(hr)) return hr;

		hr = pD3DDevice->CreateShaderResourceView(pEarthTexture[n], &srvDesc, &pEarthSRV[n]);
		if (FAILED(hr)) return hr;
	}

	return hr;
}
#endif	// EARTH_MONTH_IMAGE

HRESULT CreatePlanetTexture(ID3D11Device* pDevice, Body& b) {

	#include "image/sun.h"
	#include "image/mercury.h"
	#include "image/venus.h"
	#include "image/earth.h"
	#include "image/mars.h"
	#include "image/jupiter.h"
	#include "image/saturn.h"
	#include "image/uranus.h"
	#include "image/neptune.h"
	#include "image/pluto.h"
	#include "image/ceres.h"
	#include "image/halley.h"

	#include "image/moon.h"
	#include "image/phobos.h"
	#include "image/deimos.h"
	#include "image/io.h"
	#include "image/europa.h"
	#include "image/ganymede.h"
	#include "image/callisto.h"
	#include "image/mimas.h"
	#include "image/enceladus.h"
	#include "image/dione.h"
	#include "image/rhea.h"
	#include "image/titan.h"
	#include "image/iapetus.h"
	#include "image/charon.h"
	#include "image/haumea.h"

	static const struct {
		const wchar_t* name;
		const unsigned char* image;
	} planet_image[] = {
		{	L"Sun",			sun_img			},
		{	L"Mercury",		mercury_img		},
		{	L"Venus",		venus_img		},
		{	L"Earth",		earth_img		},
		{	L"Mars",		mars_img		},
		{	L"Jupiter",		jupiter_img		},
		{	L"Saturn",		saturn_img		},
		{	L"Uranus",		uranus_img		},
		{	L"Neptune",		neptune_img		},
		{	L"Pluto",		pluto_img		},
		{	L"Ceres",		ceres_img		},
		{	L"Halley",		halley_img		},
		{	L"Moon",		moon_img		},
		{	L"Phobos",		phobos_img		},
		{	L"Deimos",		deimos_img		},
		{	L"Io",			io_img			},
		{	L"Europa",		europa_img		},
		{	L"Ganymede",	ganymede_img	},
		{	L"Callisto",	callisto_img	},
		//{	L"Pan",			_img		},
		//{	L"Daphnis",		_img		},
		//{	L"Atlas",		_img		},
		//{	L"Prometheus",	_img		},
		//{	L"Pandora",		_img		},
		{	L"Mimas",		mimas_img		},
		{	L"Enceladus",	enceladus_img	},
		{	L"Dione",		dione_img		},
		{	L"Rhea",		rhea_img		},
		{	L"Titan",		titan_img		},
		{	L"Iapetus",		iapetus_img		},
		//{	L"Miranda",		_img		},
		//{	L"Ariel",		_img		},
		//{	L"Umbriel",		_img		},
		//{	L"Titania",		_img		},
		//{	L"Oberon",		_img		},
		//{	L"Triton",		_img		},
		//{	L"Nereid",		_img		},
		{	L"Charon",		charon_img	},
		{	nullptr,		nullptr		},
	};

	HRESULT hr = SEVERITY_SUCCESS;
	const unsigned char* image = haumea_img;

	for (int n = 0; planet_image[n].name != nullptr; n++) {
		if (b.name == planet_image[n].name) {
			image = planet_image[n].image;
			break;
		}
	}

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 256;
	desc.Height = 128;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc = { 1, 0 };
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = image;
	initData.SysMemPitch = 4 * 256;

	hr = pDevice->CreateTexture2D(&desc, &initData, &b.pPlanetTexture);
	if (FAILED(hr)) return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(b.pPlanetTexture, &srvDesc, &b.pPlanetSRV);
	if (FAILED(hr)) return hr;

	return hr;
}
#endif	// DIRECT3D_SWAP