module ConnectRequests
  class ConfirmationService
    def initialize(connect_request)
      @connect_request = connect_request
    end

    def execute
      send_mails_for_confirmed
      save_confirmation_time!
      create_faculty_connect_session_rating_job

      ConnectRequests::CreateCalendarEventService.new(@connect_request).execute
      create_faculty_connect_session_reminder_job
    end

    private

    def send_mails_for_confirmed
      FacultyMailer.connect_request_confirmed(@connect_request).deliver_later
      StartupMailer.connect_request_confirmed(@connect_request).deliver_later
    end

    def save_confirmation_time!
      @connect_request.update!(confirmed_at: Time.zone.now)
    end

    def create_faculty_connect_session_rating_job
      FacultyConnectSessionRatingJob.set(wait_until: connect_slot.slot_at + 45.minutes).perform_later(@connect_request.id)
    end

    def create_faculty_connect_session_reminder_job
      FacultyConnectSessionReminderJob.set(wait_until: connect_slot.slot_at - 30.minutes).perform_later(@connect_request.id)
    end

    def connect_slot
      @connect_slot ||= @connect_request.connect_slot
    end
  end
end