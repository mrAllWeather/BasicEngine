#include "Camera.h"

Camera::Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
{
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
}

Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
{
	this->Position = glm::vec3(posX, posY, posZ);
	this->WorldUp = glm::vec3(upX, upY, upZ);
	this->Yaw = yaw;
	this->Pitch = pitch;
	this->updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
{
	
	GLfloat velocity = this->MovementSpeed * deltaTime;
	std::cout << "V:\t" << velocity << std::endl;

	if (direction == FORWARD)
		this->Position += this->Front * velocity;
	if (direction == BACKWARD)
		this->Position -= this->Front * velocity;
	if (direction == LEFT)
		this->Position -= this->Right * velocity;
	if (direction == RIGHT)
		this->Position += this->Right * velocity;
}

void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
{
	xoffset *= this->MouseSensitivity;
	yoffset *= this->MouseSensitivity;

	this->Yaw += xoffset;
	this->Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (this->Pitch > 89.0f)
			this->Pitch = 89.0f;
		if (this->Pitch < -89.0f)
			this->Pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Eular angles
	this->updateCameraVectors();
}

void Camera::LookAtObject(GLboolean constrainPitch = true)
{
	if (LookAtFocus)
	{
		// https://gamedev.stackexchange.com/questions/112565/finding-pitch-yaw-values-from-lookat-vector
		glm::vec3 direction = glm::normalize(*LookAtFocus - this->Position);
		this->Pitch = glm::degrees(glm::asin(direction.y));
		this->Yaw = glm::degrees(-glm::atan(direction.x, direction.z)) + 90.0f;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (!constrainPitch)
		{
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}
}

void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	front.y = sin(glm::radians(this->Pitch));
	front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	this->Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	this->Up = glm::normalize(glm::cross(this->Right, this->Front));
}

void Camera::CircleObject(GLfloat xoffset, GLfloat yoffset)
{
	if (CircleFocus)
	{
		// We wish to circle the focused object at a set distance
		// https://www.opengl.org/discussion_boards/showthread.php/176504-Orbit-around-object

		
		this->Theta = this->Theta + xoffset; // horizontal movement
		this->Phi = this->Phi + yoffset; // vertical movement

		// We take a offset to allow us control over where the camera is in relation to the object before circling
		// Will cause clipping if offset is greater than size of item
		this->Position.x = this->WorldPositionOffset.x + this->CircleFocus->x + CircleRadius*glm::cos(this->Phi)*glm::sin(this->Theta);
		this->Position.y = this->WorldPositionOffset.y + this->CircleFocus->y + CircleRadius*glm::sin(this->Phi)*glm::cos(this->Theta);
		this->Position.z = this->WorldPositionOffset.z + this->CircleFocus->z + CircleRadius*glm::cos(this->Theta);

	}
}

void Camera::tick()
{
	this->LookAtObject(true);
}

void Camera::SetLookFocus(glm::vec3* focus)
{
	this->LookAtFocus = focus;
	LookAtObject(true);
}

void Camera::SetCircleFocus(glm::vec3* focus, float radius, glm::vec3 offset)
{
	// If we aren't currently circling something, save where we were when we started circling. 
	if (!this->CircleFocus)
		this->SavedPosition = this->Position;

	this->WorldPositionOffset = offset;
	this->CircleFocus = focus;
	this->CircleRadius = radius;

	CircleObject(0.0, 0.0);
	LookAtObject(true);
}

void Camera::RemoveCircleFocus()
{
	this->CircleFocus = NULL;
	// Return to our original camera position
	this->Position = this->SavedPosition;
}
