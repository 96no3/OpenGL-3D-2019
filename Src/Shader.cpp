/**
* @file Shader.cpp
*/
#include "Shader.h"
#include "Geometry.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <glm/gtc/matrix_transform.hpp>

/**
* シェーダーに関する機能を格納する名前空間.
*/
namespace Shader {
	/**
	* シェーダー・コードをコンパイルする.
	*
	* @param type シェーダーの種類.
	* @param string シェーダー・コードへのポインタ.
	*
	* @retval 0より大きい 作成したシェーダー・オブジェクト.
	* @retval 0         シェーダー・オブジェクトの作成に失敗.
	*/
	GLuint Compile(GLenum type, const GLchar* string)
	{
		if (!string)
		{
			return 0;
		}
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &string, nullptr);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		//コンパイルに失敗した場合、原因をコンソールに出力して0を返す.
		if (!compiled)
		{
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen)
			{
				std::vector<char> buf;
				buf.resize(infoLen);
				if (static_cast<int>(buf.size()) >= infoLen)
				{
					glGetShaderInfoLog(shader, infoLen, NULL, buf.data());
					std::cerr << "ERROR: シェーダーのコンパイルに失敗.\n" << buf.data() << std::endl;
				}
			}
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

	/**
	* プログラム・オブジェクトを作成する.
	*
	* @param vsCode 頂点シェーダー・コードへのポインタ.
	* @param fsCode フラグメントシェーダー・コードへのポインタ.
	*
	* @retval 0より大きい 作成したプログラム・オブジェクト.
	* @retval 0         プログラム・オブジェクトの作成に失敗.
	*/
	GLuint Build(const GLchar* vsCode, const GLchar* fsCode)
	{
		GLuint vs = Compile(GL_VERTEX_SHADER, vsCode);
		GLuint fs = Compile(GL_FRAGMENT_SHADER, fsCode);
		GLuint gs = Compile(GL_GEOMETRY_SHADER, 0);
		GLuint tcs = Compile(GL_TESS_CONTROL_SHADER, 0);
		GLuint tes = Compile(GL_TESS_EVALUATION_SHADER, 0);
		if (!vs || !fs)
		{
			return 0;
		}

		GLuint program = glCreateProgram();
		glAttachShader(program, fs);
		glDeleteShader(fs);
		glAttachShader(program, vs);
		glDeleteShader(vs);
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			GLint infoLen = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen)
			{
				std::vector<char> buf;
				buf.resize(infoLen);
				if (static_cast<int>(buf.size()) >= infoLen)
				{
					glGetProgramInfoLog(program, infoLen, NULL, buf.data());
					std::cerr << "ERROR: シェーダーのリンクに失敗.\n" << buf.data() << std::endl;
				}
			}
			glDeleteProgram(program);
			return 0;
		}
		return program;
	}

	/**
	* ファイルを読み込む.
	*
	* @param path 読み込むファイル名.
	*
	* @return 読み込んだデータ.
	*/
	std::vector<GLchar> ReadFile(const char* path)
	{
		std::basic_ifstream<GLchar> ifs;
		ifs.open(path, std::ios_base::binary);
		if (!ifs.is_open())
		{
			std::cerr << "ERROR: " << path << " を開けません.\n";
			return {};
		}
		ifs.seekg(0, std::ios_base::end);
		const size_t length = (size_t)ifs.tellg();
		ifs.seekg(0, std::ios_base::beg);
		std::vector<GLchar> buf(length);
		ifs.read(buf.data(), length);
		buf.push_back('\0');
		return buf;
	}

	/**
	* ファイルからプログラム・オブジェクトを作成する.
	*
	* @param vsPath 頂点シェーダー・ファイル名.
	* @param fsPath フラグメントシェーダー・ファイル名.
	*
	* @return 作成したプログラム・オブジェクト.
	*/
	GLuint BuildFromFile(const GLchar* vsPath, const GLchar* fsPath)
	{
		const std::vector<GLchar> vsCode = ReadFile(vsPath);
		const std::vector<GLchar> fsCode = ReadFile(fsPath);
		return Build(vsCode.data(), fsCode.data());
	}

	/**
	* ライトリストを初期化する.
	*
	* 全ての光源の明るさを0にする.
	*/
	void LightList::Init()
	{
		ambient.color = glm::vec3(0);
		directional.color = glm::vec3(0);
		for (int i = 0; i < 8; ++i)
		{
			point.color[i] = glm::vec3(0);
		}
		for (int i = 0; i < 4; ++i)
		{
			spot.color[i] = glm::vec3(0);
		}
	}

	/**
	* コンストラクタ.
	*/
	Program::Program()
	{
		lights.Init();
	}

	/**
	* コンストラクタ.
	*
	* @param programId プログラム・オブジェクトのID.
	*/
	Program::Program(GLuint programId)
	{
		lights.Init();
		Reset(programId);
	}

	/**
	* デストラクタ.
	*
	* プログラム・オブジェクトを削除する.
	*/
	Program::~Program()
	{
		if (id)
		{
			glDeleteProgram(id);
		}
	}

	/**
	* プログラム・オブジェクトを設定する.
	*
	* @param id プログラム・オブジェクトのID.
	*/
	void Program::Reset(GLuint programId)
	{
		glDeleteProgram(id);
		id = programId;
		if (id == 0)
		{
			locMatMVP = -1;
			locMatMV = -1;
			locPointLightPos = -1;
			locPointLightCol = -1;
			locDirLightDir = -1;
			locDirLightCol = -1;
			locAmbLightCol = -1;
			locSpotLightPos = -1;
			locSpotLightDir = -1;
			locSpotLightCol = -1;

			locMetallic = -1;
			locAlbedo = -1;
			locRoughness = -1;
			return;
		}

		locMatMVP = glGetUniformLocation(id, "matMVP");
		locMatMV = glGetUniformLocation(id, "matMV");
		locMatNormal = glGetUniformLocation(id, "matNormal");

		locPointLightPos = glGetUniformLocation(id, "pointLight.posAndCutOff");
		locPointLightCol = glGetUniformLocation(id, "pointLight.color");
		locPointLightDis = glGetUniformLocation(id, "pointLight.distance");

		locDirLightDir = glGetUniformLocation(id, "directionalLight.direction");
		locDirLightCol = glGetUniformLocation(id, "directionalLight.color");

		locAmbLightCol = glGetUniformLocation(id, "ambientLight.color");

		locSpotLightPos = glGetUniformLocation(id, "spotLight.posAndInnerCutOff");
		locSpotLightDir = glGetUniformLocation(id, "spotLight.dirAndCutOff");
		locSpotLightCol = glGetUniformLocation(id, "spotLight.color");
		locSpotLightDis = glGetUniformLocation(id, "spotLight.distance");

		locMetallic = glGetUniformLocation(id, "metallic");
		locRoughness = glGetUniformLocation(id, "roughness");
		locAlbedo = glGetUniformLocation(id, "albedo");


		const GLint matMVPLoc = glGetUniformLocation(id, "matMVP");
		if (matMVPLoc < 0)
		{
			std::cerr << "ERROR: uniform変数’matMVP’が見つかりません.\n";
			return;
		}
		const GLint texColorLoc = glGetUniformLocation(id, "texColor");
		if (texColorLoc >= 0)
		{
			glUseProgram(id);
			glUniform1i(texColorLoc, 0);
			glUseProgram(0);
		}
	}

	/**
	* プログラム・オブジェクトが設定されているか調べる.
	*
	* @retval true  設定されていない.
	* @retval false 設定されている.
	*/
	bool Program::IsNull() const
	{
		return id == 0;
	}

	/**
	* プログラム・オブジェクトをグラフィックス・パイプラインに割り当てる.
	*/
	void Program::Use()
	{
		if (id)
		{
			glUseProgram(id);
		}
	}

	/**
	* 描画に使用するテクスチャを設定する.
	*
	* @param unitNo 設定するテクスチャ・イメージ・ユニットの番号(0～).
	* @param texId  設定するテクスチャのID.
	*/
	void Program::BindTexture(GLuint unitNo, GLuint texId)
	{
		glActiveTexture(GL_TEXTURE0 + unitNo);
		glBindTexture(GL_TEXTURE_2D, texId);
	}

	/**
	* 描画に使われるライトを設定する.
	*
	* @param lights 設定するライト.
	*/
	void Program::SetLightList(const LightList & lights)
	{
		this->lights = lights;
		// ライトの色情報をGPUメモリに転送する.
		if (locAmbLightCol >= 0)
		{
			glUniform3fv(locAmbLightCol, 1, &lights.ambient.color.x);
		}
		if (locDirLightCol >= 0)
		{
			glUniform3fv(locDirLightCol, 1, &lights.directional.color.x);
		}
		if (locPointLightCol >= 0)
		{
			glUniform3fv(locPointLightCol, 8, &lights.point.color[0].x);
		}
		if (locSpotLightCol >= 0)
		{
			glUniform3fv(locSpotLightCol, 8, &lights.spot.color[0].x);
		}
	}

	/**
	* 描画に使われるビュー・プロジェクション行列を設定する.
	*
	* @param matVP 設定するビュー・プロジェクション行列.
	*/
	void Program::SetViewMatrix(const glm::mat4 & matV)
	{
		this->matView = matV;
	}

	/**
	* 描画に使われるビュー・プロジェクション行列を設定する.
	*
	* @param matVP 設定するビュー・プロジェクション行列.
	*/
	void Program::SetProjectionMatrix(const glm::mat4 & matP)
	{
		this->matProjection = matP;
	}

	/**
	* 描画に使われるビュー・プロジェクション行列を設定する.
	*
	* @param matVP 設定するビュー・プロジェクション行列.
	*/
	void Program::SetViewProjectionMatrix(const glm::mat4& matVP)
	{
		this->matProjection = matVP;
		if (locMatMVP >= 0) {
			glUniformMatrix4fv(locMatMVP, 1, GL_FALSE, &matVP[0][0]);
		}
	}

	/**
	* メッシュを描画する.
	*
	* @param mesh      描画するメッシュ.
	* @param translate平行移動量.
	* @param rotate    回転角度(ラジアン).
	* @param scale     拡大縮小率(1=等倍, 0.5=1/2倍, 2.0=2倍).
	*
	* この関数を使う前に、Use()を実行しておくこと.
	*/
	void Program::Draw(const Mesh & mesh, const glm::vec3 & translate, const glm::vec3 & rotate, const glm::vec3 & scale)
	{
		if (id == 0)
		{
			return;
		}

		// モデル行列を計算する.
		const glm::mat4 matScale = glm::scale(glm::mat4(1), scale);
		const glm::mat4 matRotateZ = glm::rotate(glm::mat4(1), rotate.z, glm::vec3(0, 0, -1));
		const glm::mat4 matRotateXZ = glm::rotate(matRotateZ, rotate.x, glm::vec3(1, 0, 0));
		const glm::mat4 matRotateYXZ = glm::rotate(matRotateXZ, rotate.y, glm::vec3(0, 1, 0));
		const glm::mat4 matTranslate = glm::translate(glm::mat4(1), translate);
		const glm::mat4 matModel = matTranslate * matRotateYXZ * matScale;

		// モデル・ビュー・プロジェクション行列を計算し、GPUメモリに転送する.
		const glm::mat4 matMVP = matProjection * matView * matModel;
		const glm::mat4 matMV = matView * matModel;
		glUniformMatrix4fv(locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
		glUniformMatrix4fv(locMatMV, 1, GL_FALSE, &matMV[0][0]);
		glUniformMatrix3fv(locMatNormal, 1, GL_FALSE, &matMV[0][0]);

		// 指向性ライトの向きをモデル座標系に変換してGPUメモリに転送する.
		const glm::mat3 matInvRotate = glm::transpose(glm::inverse(glm::mat3(matRotateYXZ)));
		if (locDirLightDir >= 0)
		{
			const glm::vec3 dirLightDirOnModel = matInvRotate * lights.directional.direction;
			glUniform3fv(locDirLightDir, 1, &dirLightDirOnModel.x);
		}

		// モデル座標系におけるポイントライトの座標を計算し、GPUメモリに転送する.
		const glm::mat4 matInvModel = glm::inverse(matModel);
		if (locPointLightPos >= 0)
		{
			glm::vec3 pointLightPosOnModel[8];
			for (int i = 0; i < 8; ++i)
			{
				pointLightPosOnModel[i] = matInvModel * lights.point.posAndCutOff[i];
			}
			glUniform3fv(locPointLightPos, 8, &pointLightPosOnModel[0].x);
			glUniform1f(locPointLightDis, lights.point.distance[0]);
		}

		// モデル座標系におけるスポットライトの座標を計算し、GPUメモリに転送する.
		if (locSpotLightDir >= 0 && locSpotLightPos)
		{
			glm::vec4 spotLightDirOnModel[8];
			glm::vec4 spotLightPosOnModel[8];
			for (int i = 0; i < 8; ++i)
			{
				const glm::vec3 invDir = matInvRotate * glm::vec3(lights.spot.dirAndCutOff[i]);
				spotLightDirOnModel[i] = glm::vec4(invDir, lights.spot.dirAndCutOff[i].w);
				const glm::vec3 pos = lights.spot.posAndInnerCutOff[i];
				spotLightPosOnModel[i] = matInvModel * glm::vec4(pos, 1);
				spotLightPosOnModel[i].w = lights.spot.posAndInnerCutOff[i].w;
			}
			glUniform4fv(locSpotLightDir, 8, &spotLightDirOnModel[0].x);
			glUniform4fv(locSpotLightPos, 8, &spotLightPosOnModel[0].x);
			glUniform1f(locSpotLightDis, lights.spot.distance[0]);
		}
		{
			glUniform1f(locMetallic, lights.metallic);
			glUniform1f(locRoughness, lights.roughness);
			glUniform3fv(locAlbedo, 1, &lights.albedo.x);
		}

		// メッシュを描画する.
		glDrawElementsBaseVertex(mesh.mode, mesh.count, GL_UNSIGNED_SHORT, mesh.indices, mesh.baseVertex);

	}

	/**
	* 2Dテクスチャを作成する
	*
	* @param path テクスチャファイル名
	*
	* @return 作成したテクスチャオブジェクト
	*/
	ProgramPtr Program::Create(const char* vspath, const char* fspath) {
		return std::make_shared<Program>(BuildFromFile(vspath, fspath));
	}

} // Shader namespace