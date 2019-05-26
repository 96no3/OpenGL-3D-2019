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
* �V�F�[�_�[�Ɋւ���@�\���i�[���閼�O���.
*/
namespace Shader {
	/**
	* �V�F�[�_�[�E�R�[�h���R���p�C������.
	*
	* @param type �V�F�[�_�[�̎��.
	* @param string �V�F�[�_�[�E�R�[�h�ւ̃|�C���^.
	*
	* @retval 0���傫�� �쐬�����V�F�[�_�[�E�I�u�W�F�N�g.
	* @retval 0         �V�F�[�_�[�E�I�u�W�F�N�g�̍쐬�Ɏ��s.
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
		//�R���p�C���Ɏ��s�����ꍇ�A�������R���\�[���ɏo�͂���0��Ԃ�.
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
					std::cerr << "ERROR: �V�F�[�_�[�̃R���p�C���Ɏ��s.\n" << buf.data() << std::endl;
				}
			}
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

	/**
	* �v���O�����E�I�u�W�F�N�g���쐬����.
	*
	* @param vsCode ���_�V�F�[�_�[�E�R�[�h�ւ̃|�C���^.
	* @param fsCode �t���O�����g�V�F�[�_�[�E�R�[�h�ւ̃|�C���^.
	*
	* @retval 0���傫�� �쐬�����v���O�����E�I�u�W�F�N�g.
	* @retval 0         �v���O�����E�I�u�W�F�N�g�̍쐬�Ɏ��s.
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
					std::cerr << "ERROR: �V�F�[�_�[�̃����N�Ɏ��s.\n" << buf.data() << std::endl;
				}
			}
			glDeleteProgram(program);
			return 0;
		}
		return program;
	}

	/**
	* �t�@�C����ǂݍ���.
	*
	* @param path �ǂݍ��ރt�@�C����.
	*
	* @return �ǂݍ��񂾃f�[�^.
	*/
	std::vector<GLchar> ReadFile(const char* path)
	{
		std::basic_ifstream<GLchar> ifs;
		ifs.open(path, std::ios_base::binary);
		if (!ifs.is_open())
		{
			std::cerr << "ERROR: " << path << " ���J���܂���.\n";
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
	* �t�@�C������v���O�����E�I�u�W�F�N�g���쐬����.
	*
	* @param vsPath ���_�V�F�[�_�[�E�t�@�C����.
	* @param fsPath �t���O�����g�V�F�[�_�[�E�t�@�C����.
	*
	* @return �쐬�����v���O�����E�I�u�W�F�N�g.
	*/
	GLuint BuildFromFile(const GLchar* vsPath, const GLchar* fsPath)
	{
		const std::vector<GLchar> vsCode = ReadFile(vsPath);
		const std::vector<GLchar> fsCode = ReadFile(fsPath);
		return Build(vsCode.data(), fsCode.data());
	}

	/**
	* ���C�g���X�g������������.
	*
	* �S�Ă̌����̖��邳��0�ɂ���.
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
	* �R���X�g���N�^.
	*/
	Program::Program()
	{
		lights.Init();
	}

	/**
	* �R���X�g���N�^.
	*
	* @param programId �v���O�����E�I�u�W�F�N�g��ID.
	*/
	Program::Program(GLuint programId)
	{
		lights.Init();
		Reset(programId);
	}

	/**
	* �f�X�g���N�^.
	*
	* �v���O�����E�I�u�W�F�N�g���폜����.
	*/
	Program::~Program()
	{
		if (id)
		{
			glDeleteProgram(id);
		}
	}

	/**
	* �v���O�����E�I�u�W�F�N�g��ݒ肷��.
	*
	* @param id �v���O�����E�I�u�W�F�N�g��ID.
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
			std::cerr << "ERROR: uniform�ϐ��fmatMVP�f��������܂���.\n";
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
	* �v���O�����E�I�u�W�F�N�g���ݒ肳��Ă��邩���ׂ�.
	*
	* @retval true  �ݒ肳��Ă��Ȃ�.
	* @retval false �ݒ肳��Ă���.
	*/
	bool Program::IsNull() const
	{
		return id == 0;
	}

	/**
	* �v���O�����E�I�u�W�F�N�g���O���t�B�b�N�X�E�p�C�v���C���Ɋ��蓖�Ă�.
	*/
	void Program::Use()
	{
		if (id)
		{
			glUseProgram(id);
		}
	}

	/**
	* �`��Ɏg�p����e�N�X�`����ݒ肷��.
	*
	* @param unitNo �ݒ肷��e�N�X�`���E�C���[�W�E���j�b�g�̔ԍ�(0�`).
	* @param texId  �ݒ肷��e�N�X�`����ID.
	*/
	void Program::BindTexture(GLuint unitNo, GLuint texId)
	{
		glActiveTexture(GL_TEXTURE0 + unitNo);
		glBindTexture(GL_TEXTURE_2D, texId);
	}

	/**
	* �`��Ɏg���郉�C�g��ݒ肷��.
	*
	* @param lights �ݒ肷�郉�C�g.
	*/
	void Program::SetLightList(const LightList & lights)
	{
		this->lights = lights;
		// ���C�g�̐F����GPU�������ɓ]������.
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
	* �`��Ɏg����r���[�E�v���W�F�N�V�����s���ݒ肷��.
	*
	* @param matVP �ݒ肷��r���[�E�v���W�F�N�V�����s��.
	*/
	void Program::SetViewMatrix(const glm::mat4 & matV)
	{
		this->matView = matV;
	}

	/**
	* �`��Ɏg����r���[�E�v���W�F�N�V�����s���ݒ肷��.
	*
	* @param matVP �ݒ肷��r���[�E�v���W�F�N�V�����s��.
	*/
	void Program::SetProjectionMatrix(const glm::mat4 & matP)
	{
		this->matProjection = matP;
	}

	/**
	* �`��Ɏg����r���[�E�v���W�F�N�V�����s���ݒ肷��.
	*
	* @param matVP �ݒ肷��r���[�E�v���W�F�N�V�����s��.
	*/
	void Program::SetViewProjectionMatrix(const glm::mat4& matVP)
	{
		this->matProjection = matVP;
		if (locMatMVP >= 0) {
			glUniformMatrix4fv(locMatMVP, 1, GL_FALSE, &matVP[0][0]);
		}
	}

	/**
	* ���b�V����`�悷��.
	*
	* @param mesh      �`�悷�郁�b�V��.
	* @param translate���s�ړ���.
	* @param rotate    ��]�p�x(���W�A��).
	* @param scale     �g��k����(1=���{, 0.5=1/2�{, 2.0=2�{).
	*
	* ���̊֐����g���O�ɁAUse()�����s���Ă�������.
	*/
	void Program::Draw(const Mesh & mesh, const glm::vec3 & translate, const glm::vec3 & rotate, const glm::vec3 & scale)
	{
		if (id == 0)
		{
			return;
		}

		// ���f���s����v�Z����.
		const glm::mat4 matScale = glm::scale(glm::mat4(1), scale);
		const glm::mat4 matRotateZ = glm::rotate(glm::mat4(1), rotate.z, glm::vec3(0, 0, -1));
		const glm::mat4 matRotateXZ = glm::rotate(matRotateZ, rotate.x, glm::vec3(1, 0, 0));
		const glm::mat4 matRotateYXZ = glm::rotate(matRotateXZ, rotate.y, glm::vec3(0, 1, 0));
		const glm::mat4 matTranslate = glm::translate(glm::mat4(1), translate);
		const glm::mat4 matModel = matTranslate * matRotateYXZ * matScale;

		// ���f���E�r���[�E�v���W�F�N�V�����s����v�Z���AGPU�������ɓ]������.
		const glm::mat4 matMVP = matProjection * matView * matModel;
		const glm::mat4 matMV = matView * matModel;
		glUniformMatrix4fv(locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
		glUniformMatrix4fv(locMatMV, 1, GL_FALSE, &matMV[0][0]);
		glUniformMatrix3fv(locMatNormal, 1, GL_FALSE, &matMV[0][0]);

		// �w�������C�g�̌��������f�����W�n�ɕϊ�����GPU�������ɓ]������.
		const glm::mat3 matInvRotate = glm::transpose(glm::inverse(glm::mat3(matRotateYXZ)));
		if (locDirLightDir >= 0)
		{
			const glm::vec3 dirLightDirOnModel = matInvRotate * lights.directional.direction;
			glUniform3fv(locDirLightDir, 1, &dirLightDirOnModel.x);
		}

		// ���f�����W�n�ɂ�����|�C���g���C�g�̍��W���v�Z���AGPU�������ɓ]������.
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

		// ���f�����W�n�ɂ�����X�|�b�g���C�g�̍��W���v�Z���AGPU�������ɓ]������.
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

		// ���b�V����`�悷��.
		glDrawElementsBaseVertex(mesh.mode, mesh.count, GL_UNSIGNED_SHORT, mesh.indices, mesh.baseVertex);

	}

	/**
	* 2D�e�N�X�`�����쐬����
	*
	* @param path �e�N�X�`���t�@�C����
	*
	* @return �쐬�����e�N�X�`���I�u�W�F�N�g
	*/
	ProgramPtr Program::Create(const char* vspath, const char* fspath) {
		return std::make_shared<Program>(BuildFromFile(vspath, fspath));
	}

} // Shader namespace